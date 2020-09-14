/******************************************************************************
 * FAT.C
 * 
 * DESCRIPTION
 * Defines functions to interact with a FAT32 formatted volume.
 * 
 * 
 * "PUBLIC" FUNCTIONS
 * (1) uint8_t   SetFatCurrentDirectory(
 *                  FatCurrentDirectory *currentDirectory, 
 *                  char *newDirectory)
 * (2) uint8_t   PrintFatCurrentDirectoryContents(
 *                  FatCurrentDirectory *currentDirectory, 
 *                  uint8_t FLAG)
 * (3) uint8_t   PrintFatFileContents(
 *                  FatCurrentDirectory *currentDirectory,
 *                  char *fileName)
 * (4)  void     PrintFatError(uint8_t err)
 * (5)  uint16_t GetFatBytsPerSec()
 * (6)  uint8_t  GetFatSecPerClus()
 * (7)  uint16_t GetFatRsvdSecCnt()
 * (8)  uint8_t  GetFatNumFATs()
 * (9)  uint32_t GetFatFATSz32()
 * (10) uint32_t GetFatRootClus()
 * 
 * Author: Joshua Fain
 * Date: 9/12/2020
 *****************************************************************************/



#include <string.h>
#include <avr/io.h>
#include "../includes/fattosd.h"
#include "../includes/fat.h"
#include "../includes/prints.h"
#include "../includes/usart.h"



/******************************************************************************
 *                        "PRIVATE" FUNCTION DECLARATIONS
******************************************************************************/
uint32_t pvt_GetNextCluster(uint32_t CurrentCluster);
void pvt_PrintEntryFields(uint8_t *byte, uint16_t entry, uint8_t FLAG);
void pvt_PrintShortNameAndType(uint8_t *byte, uint16_t entry, uint8_t attr);
void pvt_PrintFatFile(uint16_t entry, uint8_t *byte);



/******************************************************************************
 *                         "PUBLIC" FUNCTION DEFINITIONS
******************************************************************************/

// Set currentDirectory to newDirectory if found.
// Return a Fat Error Flag
uint8_t SetFatCurrentDirectory(
                FatCurrentDirectory *currentDirectory, 
                char *newDirectory)
{
    uint8_t ndlen = strlen(newDirectory);
    
    // *** SECTION: Verify newDirectory is a legal directory name ***
    if ((strcmp(newDirectory,"") == 0) ) return INVALID_DIR_NAME;
    if ( newDirectory[0] == ' ') return INVALID_DIR_NAME;
    
    for (uint8_t k = 0; k < ndlen; k++)
    {       
        if( ( newDirectory[k] == 92 /* '\' */) || 
            ( newDirectory[k] == '/' ) ||
            ( newDirectory[k] == ':' ) ||
            ( newDirectory[k] == '*' ) ||
            ( newDirectory[k] == '?' ) ||
            ( newDirectory[k] == '"' ) ||
            ( newDirectory[k] == '<' ) ||
            ( newDirectory[k] == '>' ) ||
            ( newDirectory[k] == '|' )   )
        {
            return INVALID_DIR_NAME;
        }
    }
    uint8_t allSpaces = 1;
    for (uint8_t k = 0; k < ndlen; k++) 
    { 
        if(newDirectory[k] != ' ') {  allSpaces = 0;  break; }
    }
    if ( allSpaces == 1 ) return INVALID_DIR_NAME;
    // *** END SECTION: legal name verification ***


    uint16_t bytsPerSec = GetFatBytsPerSec(); // Must be 512
    uint8_t  secPerClus = GetFatSecPerClus();
    uint32_t physicalSectorNumber;
    uint32_t cluster = currentDirectory->FATFirstCluster;
    uint32_t dataRegionFirstSector = GetFatRsvdSecCnt() + (GetFatNumFATs() * GetFatFATSz32());

    uint8_t currentSectorContents[bytsPerSec]; 
    uint8_t nextSectorContents[bytsPerSec];

    uint16_t shortNamePosCS = 0; //position of short name in currentSectorContents
    uint16_t shortNamePosNS = 0; //position of short name in nextSectorContents

    char    longNameStr[64];
    uint8_t longNameStrIndex = 0;

    uint8_t  attributeByte;

    // Long Name flags
    uint8_t longNameExistsFlag = 0; 
    uint8_t longNameCrossSectorFlag = 0;
    uint8_t longNameLastSectorEntryFlag = 0;
    

    // *** SECTION: if newDirectory is the current or parent directory ***
    if (!strcmp(newDirectory,".")) return SUCCESS; // newDirectory = Current Directory
    
    if(!strcmp(newDirectory,"..")) // newDirectory = Parent Dirctory
    {
        uint32_t parentDirectoryFirstCluster;

        physicalSectorNumber = dataRegionFirstSector + ( (cluster-2) * secPerClus);
        fat_ReadSingleSector( bytsPerSec * physicalSectorNumber, currentSectorContents);

        parentDirectoryFirstCluster = currentSectorContents[53];
        parentDirectoryFirstCluster <<= 8;
        parentDirectoryFirstCluster |= currentSectorContents[52];
        parentDirectoryFirstCluster <<= 8;
        parentDirectoryFirstCluster |= currentSectorContents[59];
        parentDirectoryFirstCluster <<= 8;
        parentDirectoryFirstCluster |= currentSectorContents[58];

        // current directory is root directory? Do Nothing. 
        if(currentDirectory->FATFirstCluster == GetFatRootClus()); 

        // parent directory is root directory?
        else if(parentDirectoryFirstCluster == 0)
        {
            strcpy(currentDirectory->shortName,"/");
            strcpy(currentDirectory->shortParentPath,"");
            strcpy(currentDirectory->longName,"/");
            strcpy(currentDirectory->longParentPath,"");
            currentDirectory->FATFirstCluster = GetFatRootClus();
        }

        else
        {
            //update longName/shortName currentDirectory struct members with the current parent
            //directory name by using the last directory in the longParentPath/shortParentPath.  
            
            currentDirectory->FATFirstCluster = parentDirectoryFirstCluster;
            
            char tmpSpath[64];
            char tmpLpath[64];

            strlcpy(tmpSpath,currentDirectory->shortParentPath,strlen(currentDirectory->shortParentPath)); 
            strlcpy(tmpLpath,currentDirectory->longParentPath,strlen(currentDirectory->longParentPath));
            
            char *slast = strrchr(tmpSpath, '/');
            char *llast = strrchr(tmpLpath, '/');
            
            strcpy(currentDirectory->shortName, slast+1);
            strcpy(currentDirectory->longName, llast+1);
            strlcpy(currentDirectory->shortParentPath,tmpSpath,((int)slast + 2) - (int)tmpSpath);
            strlcpy(currentDirectory->longParentPath,tmpLpath,((int)llast + 2) - (int)tmpLpath);
        }
        return SUCCESS;
    }
    // ** END SECTION : if newDirectory is current or parent directory **
    

    // ** SECTION : newDirectory is expected to be a child directory **
    do
    {
        for(int clusterSectorNumber = 0; clusterSectorNumber < secPerClus; clusterSectorNumber++)
        {         
            // get currentSectorContents[]
            physicalSectorNumber = clusterSectorNumber + dataRegionFirstSector + ( (cluster - 2) * secPerClus );
            fat_ReadSingleSector( bytsPerSec * physicalSectorNumber, currentSectorContents );

            for(int entry = 0; entry < 512; entry = entry+32)
            {
                // ensure 'entry' is pointing at correct location in sector
                if(longNameExistsFlag)  
                {
                    if (shortNamePosCS >= 480)
                    {
                        if ( entry != 0)  break;
                        else shortNamePosCS = -32;
                    }

                    if( (longNameCrossSectorFlag || longNameLastSectorEntryFlag) )
                    {
                        entry = shortNamePosNS + 32;
                        shortNamePosNS = 0;
                        longNameCrossSectorFlag = 0;
                        longNameLastSectorEntryFlag = 0;
                    }

                    else 
                    {
                        entry = shortNamePosCS + 32;
                        shortNamePosCS = 0;
                    }
                    longNameExistsFlag = 0;
                }
                
                // marked for deletion?
                if( currentSectorContents[entry] == 0xE5 );

                // all subsequent entries are empty.
                else if ( currentSectorContents[entry] == 0 ) return END_OF_DIRECTORY;
                else
                {                
                    attributeByte = currentSectorContents[entry + 11];

                    // Long Name?
                    if( (attributeByte & 0x0F) == 0x0F )
                    {
                        // confirm long name last entry flag is set for this entry
                        if( !(currentSectorContents[entry] & 0x40) ) return CORRUPT_FAT_ENTRY; 
                        else
                        {
                            longNameExistsFlag = 1;
                            for(int k = 0; k < 64; k++) longNameStr[k] = '\0';
                        
                            // number of entries required for the long name
                            int ord = 0x3F & currentSectorContents[entry];

                            shortNamePosCS = entry + (32 * ord);
                            
                            // short name is in next sector?
                            if (shortNamePosCS >= bytsPerSec)
                            {
                                // long name crosses sector boundary?
                                if (shortNamePosCS > bytsPerSec)
                                {
                                    longNameCrossSectorFlag = 1;
                                    longNameLastSectorEntryFlag = 0;
                                }

                                // entire long name is in current sector?
                                else if (shortNamePosCS == bytsPerSec)
                                {
                                    longNameCrossSectorFlag = 0;
                                    longNameLastSectorEntryFlag = 1;
                                }
                                else return CORRUPT_FAT_ENTRY;

                                //get next sector's contents
                                uint32_t nextSec;
                                if (clusterSectorNumber >= secPerClus - 1) 
                                    { nextSec = dataRegionFirstSector + ( (pvt_GetNextCluster(cluster) - 2) * secPerClus ); }
                                else nextSec = 1 + physicalSectorNumber;
                                fat_ReadSingleSector( bytsPerSec * nextSec, nextSectorContents);

                                // short name start position in the next sector
                                shortNamePosNS = shortNamePosCS - bytsPerSec;

                                attributeByte = nextSectorContents[shortNamePosNS+11];
                                
                                // If not a directory entry, move on to next entry
                                if( !(attributeByte&0x10) );

                                // shortNamePosNS points to long name entry?
                                if (attributeByte == 0x0F) return CORRUPT_FAT_ENTRY;
                                else
                                {                                                           
                                    // Long name crosses sector boundary?
                                    if(longNameCrossSectorFlag == 1 && longNameLastSectorEntryFlag == 0)
                                    {
                                        // Confirm entry preceding short name is first entry of a long name.
                                        if( (nextSectorContents[shortNamePosNS-32] & 0x0F) != 1) return CORRUPT_FAT_ENTRY;                                      
                                        else
                                        {
                                            longNameStrIndex = 0;   

                                            // load long name entry into longNameStr[]
                                            for(int i = (shortNamePosNS - 32)  ; i >= 0 ; i = i - 32)
                                            {
                                                for(int n = i + 1; n < i + 11; n++)
                                                {                                  
                                                    if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                                }

                                                for(int n = i + 14; n < i + 26; n++)
                                                {                                  
                                                    if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                                }
                                                
                                                for(int n = i + 28; n < i + 32; n++)
                                                {                                  
                                                    if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                                }            
                                            }
                                        
                                            for(int i = 480 ; i >= entry ; i = i - 32)
                                            {                                
                                                for(int n = i + 1; n < i + 11; n++)
                                                {                                  
                                                    if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                                }
                                                
                                                for(int n = i + 14; n < i + 26; n++)
                                                {   
                                                    if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                                }
                                                
                                                for(int n = i + 28; n < i + 32; n++)
                                                {                                  
                                                    if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                                }
                                            }

                                            // if match, then update currentDirectory members
                                            if(!strcmp(newDirectory,longNameStr)) 
                                            {                                                        
                                                uint32_t DIR_FstClus;
                                                DIR_FstClus = nextSectorContents[shortNamePosNS+21];
                                                DIR_FstClus <<= 8;
                                                DIR_FstClus |= nextSectorContents[shortNamePosNS+20];
                                                DIR_FstClus <<= 8;
                                                DIR_FstClus |= nextSectorContents[shortNamePosNS+27];
                                                DIR_FstClus <<= 8;
                                                DIR_FstClus |= nextSectorContents[shortNamePosNS+26];

                                                currentDirectory->FATFirstCluster = DIR_FstClus;

                                                char SN[9];                                                     
                                                for(int k = 0; k < 8; k++) SN[k] = nextSectorContents[shortNamePosNS + k];
                                                SN[8] = '\0';

                                                strcat(currentDirectory->longParentPath,currentDirectory->longName);
                                                strcat(currentDirectory->shortParentPath,currentDirectory->shortName);

                                                // if current directory is not root, append '/'
                                                if(currentDirectory->longName[0] != '/') strcat(currentDirectory->longParentPath,"/"); 
                                                strcpy(currentDirectory->longName,newDirectory);
                                                if(currentDirectory->shortName[0] != '/') strcat(currentDirectory->shortParentPath,"/");
                                                strcpy(currentDirectory->shortName,SN);

                                                return SUCCESS;
                                            }
                                        }
                                    }

                                    // all entries for long name are in current sector but short name is in next sector
                                    else if(longNameCrossSectorFlag == 0 && longNameLastSectorEntryFlag == 1)
                                    {
                                        longNameStrIndex = 0;

                                        // confirm last entry of current sector is the first entry of the long name
                                        if( (currentSectorContents[480] & 0x0F) != 1) return CORRUPT_FAT_ENTRY;
                                        else
                                        {                               
                                            // load long name entry into longNameStr[]

                                            for(int i = 480 ; i >= entry ; i = i - 32)
                                            {                                
                                                for(int n = i + 1; n < i + 11; n++)
                                                {                                  
                                                    if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                                }
                                                
                                                for(int n = i + 14; n < i + 26; n++)
                                                {   
                                                    if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                                }
                                                
                                                for(int n = i + 28; n < i + 32; n++)
                                                {                                  
                                                    if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                    else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                                }
                                            }

                                            // if match, then update currentDirectory members
                                            if(!strcmp(newDirectory,longNameStr)) 
                                            { 
                                                uint32_t DIR_FstClus;
                                                DIR_FstClus = nextSectorContents[shortNamePosNS+21];
                                                DIR_FstClus <<= 8;
                                                DIR_FstClus |= nextSectorContents[shortNamePosNS+20];
                                                DIR_FstClus <<= 8;
                                                DIR_FstClus |= nextSectorContents[shortNamePosNS+27];
                                                DIR_FstClus <<= 8;
                                                DIR_FstClus |= nextSectorContents[shortNamePosNS+26];

                                                currentDirectory->FATFirstCluster = DIR_FstClus;

                                                char SN[9];                                                     
                                                for(int k = 0; k < 8; k++) SN[k] = nextSectorContents[shortNamePosNS + k];
                                                SN[8] = '\0';

                                                strcat(currentDirectory->longParentPath,currentDirectory->longName);
                                                strcat(currentDirectory->shortParentPath,currentDirectory->shortName);

                                                // if current directory is not root, append '/' 
                                                if(currentDirectory->longName[0] != '/') strcat(currentDirectory->longParentPath,"/"); 
                                                strcpy(currentDirectory->longName,newDirectory);
                                                if(currentDirectory->shortName[0] != '/') strcat(currentDirectory->shortParentPath,"/");
                                                strcpy(currentDirectory->shortName,SN);

                                                return SUCCESS;
                                            }
                                        }
                                    }
                                    else return CORRUPT_FAT_ENTRY;
                                }
                            }

                            else // Long name exists and is entirely in current sector along with the short name
                            {   
                                attributeByte = currentSectorContents[shortNamePosCS+11];
                                
                                // If not a directory entry, move on to next entry.
                                if( !(attributeByte&0x10) );

                                // Confirm entry preceding short name is first entry of a long name.
                                if( (currentSectorContents[shortNamePosCS-32] & 0x0F) != 1) return CORRUPT_FAT_ENTRY;
                                else
                                {
                                    longNameStrIndex = 0;

                                    // load long name entry into longNameStr[]
                                    for(int i = shortNamePosCS - 32 ; i >= entry ; i = i - 32)
                                    {                                
                                        for(int n = i + 1; n < i + 11; n++)
                                        {                                  
                                            if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                            else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                        }
                                        
                                        for(int n = i + 14; n < i + 26; n++)
                                        {   
                                            if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                            else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                        }
                                        
                                        for(int n = i + 28; n < i + 32; n++)
                                        {                                  
                                            if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                            else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                        }
                                    }
                                    
                                    // if match, then update currentDirectory members
                                    if(!strcmp(newDirectory,longNameStr)) 
                                    { 
                                        uint32_t DIR_FstClus;
                                        DIR_FstClus = currentSectorContents[shortNamePosCS+21];
                                        DIR_FstClus <<= 8;
                                        DIR_FstClus |= currentSectorContents[shortNamePosCS+20];
                                        DIR_FstClus <<= 8;
                                        DIR_FstClus |= currentSectorContents[shortNamePosCS+27];
                                        DIR_FstClus <<= 8;
                                        DIR_FstClus |= currentSectorContents[shortNamePosCS+26];

                                        currentDirectory->FATFirstCluster = DIR_FstClus;
                                        
                                        char SN[9];                                    
                                        for(int k = 0; k < 8; k++)  SN[k] = currentSectorContents[shortNamePosCS + k];
                                        SN[8] = '\0';

                                        strcat(currentDirectory->longParentPath,currentDirectory->longName);
                                        strcat(currentDirectory->shortParentPath,currentDirectory->shortName);

                                        // if current directory is not root then append '/'
                                        if(currentDirectory->longName[0] != '/') strcat(currentDirectory->longParentPath,"/"); 
                                        strcpy(currentDirectory->longName,newDirectory);
                                        if(currentDirectory->shortName[0] != '/') strcat(currentDirectory->shortParentPath,"/");
                                        strcpy(currentDirectory->shortName,SN);

                                        return SUCCESS;
                                    }                                       
                                }
                            }
                        }
                    }                   

                    else  // Long Name Entry does not exist
                    {
                        attributeByte = currentSectorContents[entry+11];

                        // If not a directory entry, move on to next entry.
                        if( !(attributeByte&0x10) );

                        // newDirectory is too long for a short name
                        else if(ndlen > 8);

                        else 
                        {                   
                            char SN[9];
                    
                            char tempDir[9];
                            strcpy(tempDir,newDirectory);

                            for(int k = 0; k < ndlen; k++)  {  SN[k] = currentSectorContents[k+entry];  }
                            SN[ndlen] = '\0';

                            // if match, then update currentDirectory members
                            if(!strcmp(tempDir,SN)) 
                            { 
                                uint32_t DIR_FstClus;
                                DIR_FstClus = currentSectorContents[entry+21];
                                DIR_FstClus <<= 8;
                                DIR_FstClus |= currentSectorContents[entry+20];
                                DIR_FstClus <<= 8;
                                DIR_FstClus |= currentSectorContents[entry+27];
                                DIR_FstClus <<= 8;
                                DIR_FstClus |= currentSectorContents[entry+26];

                                currentDirectory->FATFirstCluster = DIR_FstClus;
                                
                                strcat(currentDirectory->longParentPath,currentDirectory->longName);
                                strcat(currentDirectory->shortParentPath,currentDirectory->shortName);
                                
                                // if current directory is not root then append '/'
                                if(currentDirectory->longName[0] != '/') strcat(currentDirectory->longParentPath,"/");
                                strcpy(currentDirectory->longName,newDirectory);
                                if(currentDirectory->shortName[0] != '/') strcat(currentDirectory->shortParentPath,"/");
                                strcpy(currentDirectory->shortName,SN);

                                return SUCCESS;
                            }
                        }
                    }
                }
            }
        }
    } while( ( (cluster = pvt_GetNextCluster(cluster)) != 0x0FFFFFFF ) );

    return END_OF_DIRECTORY;
    // ** END SECTION : if newDirectory is expected to be a child directory **
}


// Prints long and/or short name entries found in the current directory as well
// as prints the entry's associated fields as specified by FLAG.
// Returns a Fat Error Flag
uint8_t PrintFatCurrentDirectoryContents(
                FatCurrentDirectory *currentDirectory, 
                uint8_t FLAG)
{
    print_str("\n\rCurrent Directory: "); print_str(currentDirectory->longName);

    uint16_t bytsPerSec = GetFatBytsPerSec();  // Must be 512
    uint8_t  secPerClus = GetFatSecPerClus();
    uint32_t physicalSectorNumber;  // absolute (phyiscal) sector number
    uint32_t cluster = currentDirectory->FATFirstCluster;
    uint32_t dataRegionFirstSector = GetFatRsvdSecCnt() + (GetFatNumFATs() * GetFatFATSz32()); // Data Region First Sector 
    
    uint8_t currentSectorContents[bytsPerSec]; 
    uint8_t nextSectorContents[bytsPerSec];

    uint16_t shortNamePosCS = 0;   //position of short name in currentSectorContents
    uint16_t shortNamePosNS = 0; //position of short name in nextSectorContents

    char    longNameStr[64];
    uint8_t longNameStrIndex = 0;

    uint8_t  attributeByte;

    // long name flags. Set to 1 if true for current name
    uint8_t longNameExistsFlag = 0; 
    uint8_t longNameCrossSectorFlag = 0;
    uint8_t longNameLastSectorEntryFlag = 0;

    // Prints column headers according to flag setting
    print_str("\n\n\r");
    if(CREATION & FLAG)  print_str(" CREATION DATE & TIME,");
    if(LAST_ACCESS & FLAG)  print_str(" LAST ACCESS DATE,");
    if(LAST_MODIFIED & FLAG)  print_str(" LAST MODIFIED DATE & TIME,");
    print_str(" SIZE, TYPE, NAME");
    print_str("\n\n\r");
    

    // *** SECTION : Print entries in the current directory
    int clusCnt = 0;
    do 
    {
        clusCnt++;

        for(int clusterSectorNumber = 0; clusterSectorNumber < secPerClus; clusterSectorNumber++)
        {
            // read in next sector's contents into currentSectorContents[] array 
            physicalSectorNumber = clusterSectorNumber + dataRegionFirstSector + ( (cluster - 2) * secPerClus );
            fat_ReadSingleSector( bytsPerSec * physicalSectorNumber, currentSectorContents );

            for(int entry = 0; entry < bytsPerSec; entry = entry + 32)
            {
                // ensure 'entry' is pointing at correct location in current sector.
                if( longNameExistsFlag )
                {
                    if (shortNamePosCS >= 480 )
                    {
                        if ( entry != 0) break;
                        else shortNamePosCS = -32;
                    }

                    if( (longNameCrossSectorFlag || longNameLastSectorEntryFlag) )
                    {
                        entry = shortNamePosNS + 32;
                        shortNamePosNS = 0;
                        longNameCrossSectorFlag = 0;
                        longNameLastSectorEntryFlag = 0;
                    }

                    else 
                    {
                        entry = shortNamePosCS + 32;
                        shortNamePosCS = 0;
                    }
                    longNameExistsFlag = 0;
                }

                // this entry marked for deletion. Go to next entry.
                if( currentSectorContents[entry] == 0xE5 );

                // all subsequent entries are empty.
                else if ( currentSectorContents[entry] == 0 ) return END_OF_DIRECTORY;

                else
                {                
                    attributeByte = currentSectorContents[entry + 11];

                    //long name?
                    if( (0x0F & attributeByte) == 0x0F )
                    {
                        //confirm long name last entry flag is set
                        if( !(currentSectorContents[entry] & 0x40) ) return CORRUPT_FAT_ENTRY;
                                                
                        longNameExistsFlag = 1;
                        
                        for(int k = 0; k < 64; k++) longNameStr[k] = '\0';

                        // number of entries required by the long name
                        uint8_t ord = 0x3F & currentSectorContents[entry];

                        shortNamePosCS = entry + (32 * ord);

                        // short name in next sector?
                        if (shortNamePosCS >= bytsPerSec)
                        {
                            // long name crosses sector boundary?
                            if (shortNamePosCS > bytsPerSec)
                            {
                                longNameCrossSectorFlag = 1;
                                longNameLastSectorEntryFlag = 0;
                            }

                            // entire long name in the current sector?
                            else if (shortNamePosCS == bytsPerSec)
                            {
                                longNameCrossSectorFlag = 0;
                                longNameLastSectorEntryFlag = 1;
                            }
                            else return CORRUPT_FAT_ENTRY;

                            //get next sector's contents
                            uint32_t nextSec;
                            if (clusterSectorNumber >= secPerClus - 1) 
                            {
                                nextSec = dataRegionFirstSector + ( (pvt_GetNextCluster(cluster) - 2) * secPerClus );
                            }
                            else nextSec = 1 + physicalSectorNumber;
                            fat_ReadSingleSector( bytsPerSec * nextSec, nextSectorContents);
    
                            // short name position in the next sector
                            shortNamePosNS = shortNamePosCS - bytsPerSec;

                            attributeByte = nextSectorContents[shortNamePosNS+11];
                            
                            // shortNamePosNS points to long name entry?
                            if ( attributeByte == 0x0F ) return CORRUPT_FAT_ENTRY;

                            if ( (attributeByte & 0x02) == 0x02 && (FLAG & HIDDEN) != HIDDEN ); 
                            else
                            {                                                           
                                if( (FLAG & SHORT_NAME) != SHORT_NAME );
                                else
                                {
                                    pvt_PrintEntryFields(nextSectorContents, shortNamePosNS, FLAG);
                                    pvt_PrintShortNameAndType(nextSectorContents, shortNamePosNS, attributeByte);
                                }
                        
                                if( (FLAG & LONG_NAME) != LONG_NAME);
                                else
                                {
                                    // entries for long name cross sector boundary
                                    if(longNameCrossSectorFlag == 1 && longNameLastSectorEntryFlag == 0)
                                    {
                                        // Confirm entry preceding short name is first entry of a long name.
                                        if( ( nextSectorContents[shortNamePosNS-32] & 0x01 ) != 1 ) return CORRUPT_FAT_ENTRY;                                              

                                        pvt_PrintEntryFields(nextSectorContents, shortNamePosNS, FLAG);
                
                                        longNameStrIndex = 0;   

                                        if ( attributeByte & 0x10 ) print_str("    <DIR>    ");
                                        else print_str("   <FILE>    ");
                                        
                                        // load long name entry into longNameStr[]
                                        for(int i = (shortNamePosNS - 32)  ; i >= 0 ; i = i - 32) 
                                        {
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                            }

                                            for(int n = i + 14; n < i + 26; n++)
                                            {                                  
                                                if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                            }            
                                        }

                                        for(int i = 480 ; i >= entry ; i = i - 32)
                                        {                                
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 14; n < i + 26; n++)
                                            {   
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                        }
                                        print_str(longNameStr);
                                    }

                                    // all entries for long name are in current sector but short name is in next sector
                                    else if(longNameCrossSectorFlag == 0 && longNameLastSectorEntryFlag == 1)
                                    {
                                        longNameStrIndex = 0;

                                        // confirm last entry of current sector is the first entry of the long name
                                        if( (currentSectorContents[480] & 0x01) != 1) return CORRUPT_FAT_ENTRY;
                             
                                        pvt_PrintEntryFields(nextSectorContents, shortNamePosNS, FLAG);

                                        if(attributeByte&0x10) print_str("    <DIR>    ");
                                        else print_str("   <FILE>    ");
                                        
                                        // load long name entry into longNameStr[]
                                        for(int i = 480 ; i >= entry ; i = i - 32)
                                        {                                
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 14; n < i + 26; n++)
                                            {   
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                        }
                                        print_str(longNameStr); 
                                    }
                                    else return CORRUPT_FAT_ENTRY;
                                }
                            }
                        }
                        else // Long name exists and is entirely in current sector along with the short name
                        {   
                            attributeByte = currentSectorContents[shortNamePosCS+11];
                            
                            // shortNamePosCS points to long name entry
                            if (attributeByte == 0x0F) return CORRUPT_FAT_ENTRY;
                            if ( (attributeByte & 0x02) == 0x02 && (FLAG&HIDDEN) != HIDDEN );
                            else
                            {                   
                                if( (FLAG & SHORT_NAME) != SHORT_NAME );
                                else
                                {
                                    pvt_PrintEntryFields(currentSectorContents, shortNamePosCS, FLAG);
                                    pvt_PrintShortNameAndType(currentSectorContents, shortNamePosCS, attributeByte);
                                }

                                if( (FLAG & LONG_NAME) != LONG_NAME );
                                else
                                {
                                    // Confirm entry preceding short name is first entry of a long name.
                                    if( (currentSectorContents[shortNamePosCS-32] & 0x01) != 1) return CORRUPT_FAT_ENTRY;

                                    pvt_PrintEntryFields(currentSectorContents, shortNamePosCS, FLAG);
                                    
                                    if(attributeByte&0x10) print_str("    <DIR>    ");
                                    else print_str("   <FILE>    ");

                                    longNameStrIndex = 0;

                                    // load long name entry into longNameStr[]
                                    for(int i = shortNamePosCS - 32 ; i >= entry ; i = i - 32)
                                    {                                
                                        for(int n = i + 1; n < i + 11; n++)
                                        {                                  
                                            if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                            else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                        }
                                        
                                        for(int n = i + 14; n < i + 26; n++)
                                        {   
                                            if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                            else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                        }
                                        
                                        for(int n = i + 28; n < i + 32; n++)
                                        {                                  
                                            if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                            else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                        }
                                    }
                                    print_str(longNameStr);                                       
                                }
                            }
                        }
                    }                   
                    else  // Long Name Entry does not exist so use short name instead, regardless of SHORT_NAME FLAG setting
                    {
                        attributeByte = currentSectorContents[entry+11];

                        if ( (attributeByte & 0x02) == 0x02 && (FLAG & HIDDEN) != HIDDEN );
                        else 
                        {
                            pvt_PrintEntryFields(currentSectorContents, entry, FLAG);
                            pvt_PrintShortNameAndType(currentSectorContents, entry, attributeByte);
                        }
                    }
                }
            }
        }
    }while( ( (cluster = pvt_GetNextCluster(cluster) ) != 0x0FFFFFFF ) && clusCnt < 5);
    // *** END SECTION :  Print entries in the current directory

    return END_OF_DIRECTORY;
}


// Prints the contents of file specified by *fileName to the screen.
// Returns a Fat Error Flag
uint8_t PrintFatFileContents(
                FatCurrentDirectory *currentDirectory, 
                char *fileName)
{
    uint8_t fnlen = strlen(fileName);

    // *** SECTION: Verify newDirectory is a legal directory name ***
    if ((strcmp(fileName,"") == 0) ) return INVALID_FILE_NAME;
    if ( fileName[0] == ' ') return INVALID_FILE_NAME;
    
    for (uint8_t k = 0; k < fnlen; k++)
    {
        if( ( fileName[k] == 92 /* '\' */ ) ||
            ( fileName[k] == '/' ) ||
            ( fileName[k] == ':' ) ||
            ( fileName[k] == '*' ) ||
            ( fileName[k] == '?' ) ||
            ( fileName[k] == '"' ) ||
            ( fileName[k] == '<' ) ||
            ( fileName[k] == '>' ) ||
            ( fileName[k] == '|' )   )
        {
            return INVALID_FILE_NAME;
        }
    }

    uint8_t allSpaces = 1;
    for (uint8_t k = 0; k < fnlen; k++) 
    { 
        if(fileName[k] != ' ') {  allSpaces = 0;  break; }
    }
    if ( allSpaces == 1 ) return INVALID_FILE_NAME;
    // *** END SECTION: legal name verification ***


    uint16_t bytsPerSec = GetFatBytsPerSec();  // Must be 512
    uint8_t  secPerClus = GetFatSecPerClus();
    uint32_t physicalSectorNumber;  // absolute (phyiscal) sector number
    uint32_t cluster = currentDirectory->FATFirstCluster;
    uint32_t dataRegionFirstSector = GetFatRsvdSecCnt() + (GetFatNumFATs() * GetFatFATSz32()); // Data Region First Sector 
    
    uint8_t currentSectorContents[bytsPerSec]; 
    uint8_t nextSectorContents[bytsPerSec];

    uint16_t shortNamePosCS = 0;   //position of short name in currentSectorContents
    uint16_t shortNamePosNS = 0; //position of short name in nextSectorContents

    char    longNameStr[64];
    uint8_t longNameStrIndex = 0;

    uint8_t  attributeByte;

    // long name flags. Set to 1 if true for current name
    uint8_t longNameExistsFlag = 0; 
    uint8_t longNameCrossSectorFlag = 0;
    uint8_t longNameLastSectorEntryFlag = 0;


    // *** SECTION : Search for, and print contents of fileName if match is found ***
    int clusCnt = 0;    
    do
    {
        clusCnt++;

        for(int clusterSectorNumber = 0; clusterSectorNumber < secPerClus; clusterSectorNumber++)
        {     
            physicalSectorNumber = clusterSectorNumber + dataRegionFirstSector + ( (cluster - 2) * secPerClus );
            fat_ReadSingleSector( bytsPerSec * physicalSectorNumber, currentSectorContents );

            for(int entry = 0; entry < bytsPerSec; entry = entry + 32)
            { 
                // ensure 'entry' is pointing to correct location in current sector.
                if( longNameExistsFlag )
                {
                    if (shortNamePosCS >= 480 )
                    {
                        if ( entry != 0) break;
                        else shortNamePosCS = -32;
                    }

                    if( ( longNameCrossSectorFlag || longNameLastSectorEntryFlag) ) 
                    {
                        entry = shortNamePosNS + 32; 
                        shortNamePosNS = 0; 
                        longNameCrossSectorFlag = 0; 
                        longNameLastSectorEntryFlag = 0;
                    }

                    else 
                    {
                        entry = shortNamePosCS + 32; 
                        shortNamePosCS = 0;
                    }
                    longNameExistsFlag = 0;
                }

                // this entry marked for deletion. Go to next entry.
                if( currentSectorContents[entry] == 0xE5 );

                // all subsequent entries are empty.
                else if ( currentSectorContents[entry] == 0 ) return FILE_NOT_FOUND;
                else
                {                
                    attributeByte = currentSectorContents[entry + 11];

                    // long name?
                    if( (0x0F & attributeByte) == 0x0F)
                    {
                        // confirm last long name entry flag is set
                        if( !(currentSectorContents[entry] & 0x40) ) return CORRUPT_FAT_ENTRY;
                        else
                        {
                            longNameExistsFlag = 1;

                            for(int k = 0; k < 64; k++) longNameStr[k] = '\0';
                            
                            // number of entries required by the long name
                            uint8_t ord = 0x3F & currentSectorContents[entry]; 
                                                            
                            shortNamePosCS = entry + (32 * ord);
                            
                            // short name in next sector?
                            if (shortNamePosCS >= bytsPerSec)
                            {
                                // Long name crosses sector boundary?
                                if (shortNamePosCS > bytsPerSec)
                                {
                                    longNameCrossSectorFlag = 1;
                                    longNameLastSectorEntryFlag = 0;
                                }

                                // or Long name is entirely in current sector, but short name is in next sector.
                                else if (shortNamePosCS == 512)
                                {
                                    longNameCrossSectorFlag = 0;
                                    longNameLastSectorEntryFlag = 1;
                                }
                                else return CORRUPT_FAT_ENTRY;

                                //get next sector's contents
                                uint32_t nextSec;
                                if (clusterSectorNumber >= secPerClus - 1) nextSec = dataRegionFirstSector + ( (pvt_GetNextCluster(cluster) - 2) * secPerClus );
                                else nextSec = 1 + physicalSectorNumber;

                                // read next sector into nextSectorContents
                                fat_ReadSingleSector( bytsPerSec * nextSec, nextSectorContents);

                                // short name position in the next sector
                                shortNamePosNS = shortNamePosCS - bytsPerSec;

                                attributeByte = nextSectorContents[shortNamePosNS+11];

                                // confirm shortNamePosNS points to short name entry
                                if ( attributeByte == 0x0F ) return CORRUPT_FAT_ENTRY;

                                // Do nothing. Entry is a directory.
                                else if (attributeByte & 0x10 ); 

                                // read in long name entry
                                else 
                                {                                                           
                                    // Long name crosses sector boundary
                                    if(longNameCrossSectorFlag == 1 && longNameLastSectorEntryFlag == 0)
                                    {
                                        // confirm entry immediatedly preceding short name entry is not the first entry of a long name.
                                        if( ( nextSectorContents[shortNamePosNS-32] & 0x01 ) != 1 ) return CORRUPT_FAT_ENTRY;
                                        
                                        longNameStrIndex = 0;   
                                        
                                        // read long name into longNameStr
                                        for(int i = (shortNamePosNS - 32)  ; i >= 0 ; i = i - 32) 
                                        {
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                            }

                                            for(int n = i + 14; n < i + 26; n++)
                                            {                                  
                                                if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(nextSectorContents[n] == 0 || nextSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = nextSectorContents[n];  longNameStrIndex++;  }
                                            }            
                                        }

                                        for(int i = 480 ; i >= entry ; i = i - 32)
                                        {                                
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 14; n < i + 26; n++)
                                            {   
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                        }

                                        // print file contents if a matching entry was found
                                        if(!strcmp(fileName,longNameStr))
                                        { 
                                            pvt_PrintFatFile(shortNamePosNS, nextSectorContents); 
                                            return SUCCESS;
                                        }
                                    }

                                    // Long name is the last entry of the current sector, so its short name is the first entry of the next sector
                                    else if(longNameCrossSectorFlag == 0 && longNameLastSectorEntryFlag == 1)
                                    {
                                        longNameStrIndex = 0;

                                        // confirm last entry of current sector is the first entry of a long name
                                        if( (currentSectorContents[480] & 0x01) != 1 ) return CORRUPT_FAT_ENTRY;                                                                  
                                            
                                        // read long name into longNameStr
                                        for(int i = 480 ; i >= entry ; i = i - 32) 
                                        {                                
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 14; n < i + 26; n++)
                                            {   
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                        }

                                        // print file contents if a matching entry was found
                                        if(!strcmp(fileName,longNameStr))
                                        { 
                                            pvt_PrintFatFile(shortNamePosNS, nextSectorContents); 
                                            return SUCCESS;
                                        }                                            
                                    }
                                    else return CORRUPT_FAT_ENTRY;
                                }
                            }

                            // Long name exists and long and short name entirely in the current directory.
                            else 
                            {   
                                attributeByte = currentSectorContents[shortNamePosCS+11];
                                
                                // confirm shortNamePosCS points to a short name entry in the current sector
                                if ( attributeByte == 0x0F ) return CORRUPT_FAT_ENTRY;
                                
                                // Entry is a directory. Do nothing.
                                else if(attributeByte & 0x10); 
                                
                                else 
                                {                   
                                    // confirm entry immediatedly preceding the short name entry the first entry of a long name
                                    if( ( currentSectorContents[shortNamePosCS-32] & 0x01 ) != 1 ) return CORRUPT_FAT_ENTRY;
                                    else
                                    {
                                        longNameStrIndex = 0;

                                        // read long name into longNameStr
                                        for(int i = shortNamePosCS - 32 ; i >= entry ; i = i - 32)
                                        {                                
                                            for(int n = i + 1; n < i + 11; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 14; n < i + 26; n++)
                                            {   
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                            
                                            for(int n = i + 28; n < i + 32; n++)
                                            {                                  
                                                if(currentSectorContents[n] == 0 || currentSectorContents[n] > 126);
                                                else { longNameStr[longNameStrIndex] = currentSectorContents[n];  longNameStrIndex++; }
                                            }
                                        }
                                        
                                        // print file contents if a matching entry was found
                                        if(!strcmp(fileName,longNameStr))
                                        { 
                                            pvt_PrintFatFile(shortNamePosCS, currentSectorContents); 
                                            return SUCCESS;
                                        }                                                                                    
                                    }
                                }
                            }
                        }
                    }            

                    // Long name does not exist for current entry so
                    // check if fileName matches the short name.
                    else
                    {
                        //attributeByte = currentSectorContents[entry+11];

                        // confirm shortNamePosCS points to a short name entry in the current sector
                        //if ( attributeByte == 0x0F ) return CORRUPT_FAT_ENTRY;

                        // Entry is a directory. Do nothing.
                        if (attributeByte & 0x10 );

                        // fileName is too long to be a short name.
                        else if (fnlen > 12);
                        
                        // read in short name.
                        else 
                        {                   
                            char SN[9];
                            char EXT[4];

                            for(uint8_t k = 0; k < 9; k++) SN[k] = '\0';
                            for(uint8_t k = 0; k < 4; k++) EXT[k] = '\0'; 
                    
                            // search for location of '.', if it exists, in fileName. Exclude first position.
                            int pt = fnlen;
                            uint8_t fnExtExistsFlag = 0;
                            for(uint8_t k = 1; k < pt; k++)
                            {
                                if( k+1 >= fnlen ) { break; }
                                if( fileName[k] == '.' )  
                                {   
                                    fnExtExistsFlag = 1;
                                    pt = k; 
                                    break; 
                                }
                            }

                            char tempFN[9];
                            for(uint8_t k = 0; k < pt; k++)  { tempFN[k] = fileName[k]; }
                            for(uint8_t k = pt; k < 8; k++)  { tempFN[k] = ' '; }
                            tempFN[8] = '\0';
                            
                            for(uint8_t k = 0; k < 8; k++)  { SN[k] = currentSectorContents[k+entry]; }

                            // if name portion of short name matches then check that extensions match.
                            if(!strcmp(tempFN,SN))
                            {                                
                                uint8_t match = 0;
                                int entryEXTExistsFlag = 0;

                                for(int k = 0; k < 3; k++)  
                                {
                                    EXT[k] = currentSectorContents[entry+8+k]; 
                                    entryEXTExistsFlag = 1;
                                }

                                if( (strcmp(EXT,"   ") ) && fnExtExistsFlag ) entryEXTExistsFlag = 1;

                                if ( (!entryEXTExistsFlag) && (!fnExtExistsFlag) ) match = 1;
                                else if ( (entryEXTExistsFlag && !fnExtExistsFlag) || (!entryEXTExistsFlag && fnExtExistsFlag) ) match = 0;
                                else if (entryEXTExistsFlag && fnExtExistsFlag)
                                {
                                    char tempEXT[4];
                                    for(uint8_t k = 0; k < 3; k++) tempEXT[k] = ' ';
                                    tempEXT[3] = '\0'; 

                                    for(uint8_t k = 0; k < 3; k++)
                                    {
                                        if(fileName[k+pt+1] == '\0') break;
                                        tempEXT[k] = fileName[k+pt+1];
                                    }

                                    // Extensions match!
                                    if(!strcmp(EXT,tempEXT) ) match = 1;
                                }


                                if(match)
                                {
                                    pvt_PrintFatFile(entry, currentSectorContents);
                                    return SUCCESS;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while( ( (cluster = pvt_GetNextCluster(cluster)) != 0x0FFFFFFF ) && (clusCnt < 5) );
    // *** END SECTION : Search for, and print contents of fileName if match is found ***
 
    return FILE_NOT_FOUND; 
}


// Prints an error code returned by a fat function.
void PrintFatError(uint8_t err)
{  
    switch(err)
    {
        case SUCCESS: 
                print_str("\n\rSUCCESS");
                break;
        case END_OF_DIRECTORY:
                print_str("\n\rEND OF DIRECTORY");
                break;
        case INVALID_FILE_NAME:
                print_str("\n\rINVALID FILE NAME");
                break;
        case FILE_NOT_FOUND:
                print_str("\n\rFILE NOT FOUND");
                break;
        case INVALID_DIR_NAME:
                print_str("\n\rINVALID DIR NAME");
                break;
        case DIR_NOT_FOUND:
                print_str("\n\rDIR NOT FOUND");
                break;
        case CORRUPT_FAT_ENTRY:
                print_str("\n\rCORRUPT SECTOR");
                break;
        case END_OF_FILE:
                print_str("\n\rEND OF FILE");
                break;
        default:
                print_str("\n\rUNKNOWN ERROR");
                break;
    }
}


// **** Boot Sector/BIOS Parameter Block GET Functions ****


// Gets the number of 'bytes per sector'
// from the fat boot sector/BPB. (must be 512).
// Returns the BytsPerSector value.
uint16_t GetFatBytsPerSec()
{
    uint8_t BootSector[512];
    fat_ReadSingleSector(0,BootSector);

    //confirm boot signature is present
    if((BootSector[510] == 0x55) && (BootSector[511]==0xAA))
    {
        uint16_t BPS = BootSector[12];
                 BPS <<= 8;
                 BPS |= BootSector[11];
        
        if(BPS != 512) return CORRUPT_BOOT_SECTOR;
        return BPS;
    }
    return CORRUPT_BOOT_SECTOR;
}


// Gets the number of 'sectors per cluster' from the 
// fat boot sector/BPB. Must be a power of 2 [1, 128].
// Returns the SecPerClus value.
uint8_t GetFatSecPerClus()
{
    uint8_t BootSector[512];
    fat_ReadSingleSector(0,BootSector);

    //confirm boot signature is present
    if((BootSector[510] == 0x55) && (BootSector[511]==0xAA))
    {
        //Bytes Per Sector
        uint16_t BPS = BootSector[12];
                 BPS <<= 8;
                 BPS |= BootSector[11];
        
        if(BPS != 512) { return CORRUPT_BOOT_SECTOR; }

        // secPerClus
        return BootSector[13];
    }
    else { return CORRUPT_BOOT_SECTOR; }
}


// Gets the number of physical sectors before the first FAT from the 
// value specified by the 'reserved sector count' in the boot sector/BPB.
// Returns the RsvdSecCnt.
uint16_t GetFatRsvdSecCnt()
{
    uint8_t BootSector[512];
    fat_ReadSingleSector(0,BootSector);
    
    //confirm boot signature is present
    if((BootSector[510] == 0x55) && (BootSector[511]==0xAA))
    {
        uint16_t RSC = BootSector[15];
                 RSC <<= 8;
                 RSC |= BootSector[14];

        return RSC;
    }
    else { return CORRUPT_BOOT_SECTOR; }
}


// Gets the number of FATs from the NumFATs field in the boot sector/BPB
// Returns the NumFATs value.
uint8_t GetFatNumFATs()
{
    uint8_t BootSector[512];
    fat_ReadSingleSector(0,BootSector);
    
    //confirm boot signature is present
    if((BootSector[510] == 0x55) && (BootSector[511]==0xAA))
    {
        // Number of FATs
        return BootSector[16];
    }

    else { return CORRUPT_BOOT_SECTOR; }
}


// Gets the size of a single FAT from the 
// FATSz32 field of the fat boot sector/BPB.
//Returns the FATSz32 value.
uint32_t GetFatFATSz32()
{
    uint8_t BootSector[512];
    fat_ReadSingleSector(0,BootSector);

    //confirm boot signature is present
    if((BootSector[510] == 0x55) && (BootSector[511]==0xAA))
    {
        uint32_t FATSz32 = BootSector[39];
                 FATSz32 <<= 8;
                 FATSz32 |= BootSector[38];
                 FATSz32 <<= 8;
                 FATSz32 |= BootSector[37];
                 FATSz32 <<= 8;
                 FATSz32 |= BootSector[36];

        return FATSz32;
    }

    else { return CORRUPT_BOOT_SECTOR; }
}


// Gets the index of the first cluster of the Root Directory 
// in the FAT from the RootClus field of the boot sector/BPB.  
// Returns the RootClus value.
uint32_t GetFatRootClus()
{
    uint8_t BootSector[512];
    fat_ReadSingleSector(0,BootSector);
    
    //confirm boot signature is present
    if((BootSector[510] == 0x55) && (BootSector[511]==0xAA))
    {
        uint32_t RootClus = BootSector[47];
                 RootClus <<= 8;
                 RootClus |= BootSector[46];
                 RootClus <<= 8;
                 RootClus |= BootSector[45];
                 RootClus <<= 8;
                 RootClus |= BootSector[44];
        
        return RootClus;
    }
    
    else { return CORRUPT_BOOT_SECTOR; }
}



/******************************************************************************
 *                        "PRIVATE" FUNCTION DEFINITIONS
******************************************************************************/



/******************************************************************************
 * DESCRIPTION
 * Used by the fat functions to get the next cluster in a directory or file.
 * 
 * ARGUMENTS 
 * (1) *byte : pointer to the current directory sector loaded in memory.
 * (2) entry : entry is the first byte location of the short name in byte[].
 * (3) FLAG  : indicates which fields of the short name entry to print.
 * 
 * RETURNS
 * FAT cluster index pointed to by the current cluster. 
 * If 0xFFFFFFFF then End Of File / Directory
******************************************************************************/
uint32_t pvt_GetNextCluster(uint32_t CurrentCluster)
{
    uint32_t FATStartSector = GetFatRsvdSecCnt();
    uint16_t bytsPerSector = GetFatBytsPerSec();
    uint8_t  BytesPerClusterIndx = 4;
    uint16_t IndxdClustersPerFATSector = bytsPerSector/BytesPerClusterIndx; // = 128

    uint32_t FATClusterIndxSector = CurrentCluster/IndxdClustersPerFATSector;
    uint32_t FATClusterIndxStartByte = 4 * (CurrentCluster%IndxdClustersPerFATSector);
    uint32_t cluster = 0;

    uint32_t AbsSectorToRead = FATClusterIndxSector + FATStartSector;

    uint8_t SectorContents[bytsPerSector];

    fat_ReadSingleSector( bytsPerSector * AbsSectorToRead, SectorContents );
    cluster = SectorContents[FATClusterIndxStartByte+3];
    cluster <<= 8;
    cluster |= SectorContents[FATClusterIndxStartByte+2];
    cluster <<= 8;
    cluster |= SectorContents[FATClusterIndxStartByte+1];
    cluster <<= 8;
    cluster |= SectorContents[FATClusterIndxStartByte];

    return cluster;
}


/******************************************************************************
 * DESCRIPTION
 * private function used by PrintFatCurrentDirectoryContents() to print the
 * non-name fields of a directory entry according to FLAG.
 * 
 * ARGUMENTS 
 * (1) *byte : pointer to the current directory sector loaded in memory.
 * (2) entry : entry is the first byte location of the short name in byte[].
 * (3) FLAG  : indicates which fields of the short name entry to print.  
******************************************************************************/
void pvt_PrintEntryFields(uint8_t *byte, uint16_t entry, uint8_t FLAG)
{
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint32_t DIR_FileSize;

    if(CREATION&FLAG)
    {
        DIR_CrtTime = byte[entry+15];
        DIR_CrtTime <<= 8;
        DIR_CrtTime |= byte[entry+14];
        
        DIR_CrtDate = byte[entry+17];
        DIR_CrtDate <<= 8;
        DIR_CrtDate |= byte[entry+16];
    }

    if(LAST_ACCESS&FLAG)
    {
        DIR_LstAccDate = byte[entry+19];
        DIR_LstAccDate <<= 8;
        DIR_LstAccDate |= byte[entry+18];
    }

    if(LAST_MODIFIED&FLAG)
    {
        DIR_WrtTime = byte[entry+23];
        DIR_WrtTime <<= 8;
        DIR_WrtTime |= byte[entry+22];

        DIR_WrtDate = byte[entry+25];
        DIR_WrtDate <<= 8;
        DIR_WrtDate |= byte[entry+24];
    }

    DIR_FileSize = byte[entry+31];
    DIR_FileSize <<= 8;
    DIR_FileSize |= byte[entry+30];
    DIR_FileSize <<= 8;
    DIR_FileSize |= byte[entry+29];
    DIR_FileSize <<= 8;
    DIR_FileSize |= byte[entry+28];

    print_str("\n\r");

    if(CREATION&FLAG)
    {
        print_str("    ");
        if(((DIR_CrtDate&0x01E0)>>5)<10) print_str("0");
        print_dec((DIR_CrtDate&0x01E0)>>5);
        print_str("/");
        if((DIR_CrtDate&0x001F)<10) print_str("0");
        print_dec(DIR_CrtDate&0x001F);
        print_str("/");
        print_dec(1980+((DIR_CrtDate&0xFE00)>>9));

        print_str("  ");
        if(((DIR_CrtTime&0xF800)>>11)<10) print_str("0");
        print_dec(((DIR_CrtTime&0xF800)>>11));
        print_str(":");
        if(((DIR_CrtTime&0x07E0)>>5)<10) print_str("0");
        print_dec((DIR_CrtTime&0x07E0)>>5);
        print_str(":");
        if((2*(DIR_CrtTime&0x001F))<10) print_str("0");
        print_dec(2*(DIR_CrtTime&0x001F));
    }

    if(LAST_ACCESS&FLAG)
    {
        print_str("     ");
        if(((DIR_LstAccDate&0x01E0)>>5)<10) print_str("0");
        print_dec((DIR_LstAccDate&0x01E0)>>5);
        print_str("/");
        if((DIR_LstAccDate&0x001F)<10) print_str("0");
        print_dec(DIR_LstAccDate&0x001F);
        print_str("/");
        print_dec(1980+((DIR_LstAccDate&0xFE00)>>9));
    }


    if(LAST_MODIFIED&FLAG)
    {
        print_str("     ");
        if(((DIR_WrtDate&0x01E0)>>5)<10) print_str("0");
        print_dec((DIR_WrtDate&0x01E0)>>5);
        print_str("/");
        if((DIR_WrtDate&0x001F)<10) print_str("0");
        print_dec(DIR_WrtDate&0x001F);
        print_str("/");
        print_dec(1980+((DIR_WrtDate&0xFE00)>>9));

        print_str("  ");

        if(((DIR_WrtTime&0xF800)>>11)<10) print_str("0");
        print_dec(((DIR_WrtTime&0xF800)>>11));
        print_str(":");
        
        if(((DIR_WrtTime&0x07E0)>>5)<10) print_str("0");
        print_dec((DIR_WrtTime&0x07E0)>>5);

        print_str(":");
        if((2*(DIR_WrtTime&0x001F))<10) print_str("0");
        print_dec(2*(DIR_WrtTime&0x001F));
    }

    int div = 1000;
    print_str("     ");
         if( (DIR_FileSize/div) >= 10000000) { print_str(" "); print_dec(DIR_FileSize/div); }
    else if( (DIR_FileSize/div) >= 1000000) { print_str("  "); print_dec(DIR_FileSize/div); }
    else if( (DIR_FileSize/div) >= 100000) { print_str("   "); print_dec(DIR_FileSize/div); }
    else if( (DIR_FileSize/div) >= 10000) { print_str("    "); print_dec(DIR_FileSize/div); }
    else if( (DIR_FileSize/div) >= 1000) { print_str("     "); print_dec(DIR_FileSize/div); }
    else if( (DIR_FileSize/div) >= 100) { print_str("      "); print_dec(DIR_FileSize/div); }
    else if( (DIR_FileSize/div) >= 10) { print_str("       "); print_dec(DIR_FileSize/div); }
    else                              { print_str("        "); print_dec(DIR_FileSize/div); }        
    
    print_str("kB");

}



/******************************************************************************
 * DESCRIPTION
 * private function used by PrintFatCurrentDirectoryContents() to print the 
 * short name entry and its entry type (DIR / FILE).
 * 
 * ARGUMENTS 
 * (1) *byte : pointer to the current directory sector loaded in memory.
 * (2) entry : entry is the first byte location of the short name in byte[].
 * (3) attr  : attribute byte of the short name entry.  
*******************************************************************************/
void pvt_PrintShortNameAndType(uint8_t *byte, uint16_t entry, uint8_t attr)
{
    char SN[9]; // array to hold the short name file name string
    char EXT[5]; // array to hold the extension of the short name string

    for(int k = 0; k < 8; k++) SN[k] = ' ';
    SN[8] = '\0';
    //print_str(" ENTRY = 0x"); print_hex(entry);
    if(attr&0x10)
    {
        print_str("    <DIR>    ");
        for(int k = 0; k < 8; k++)  SN[k] = byte[entry + k];
        print_str(SN);
        print_str("    ");
    }

    else 
    {
        print_str("   <FILE>    ");

        // re-initialize extension character array;
        strcpy(EXT,".   ");

        for(int k = 1; k < 4; k++) {  EXT[k] = byte[entry + 7 + k];  }

        for(int k = 0; k < 8; k++) 
        {
            SN[k] = byte[k + entry];
            if(SN[k] == ' ') { SN[k] = '\0'; break; };
        }

        print_str(SN);
        if(strcmp(EXT,".   "))  print_str(EXT);
        for( int p = 0; p < 10 - (strlen(SN) + 2); p++ ) print_str(" ");
    }
}



/******************************************************************************
 * DESCRIPTION
 * private function used by PrintFatFileContents() to print a file's contents.
 * 
 * ARGUMENTS 
 * (1) entry : entry is the first byte location of the short name in byte[].
 * (2) *fileSector : pointer to an array loaded with the directory sector that
 *                   contains the file name entry for the file to be printed.
*******************************************************************************/
void pvt_PrintFatFile(uint16_t entry, uint8_t *fileSector)
{
    uint16_t bytsPerSec = GetFatBytsPerSec();  // Must be 512
    uint8_t  secPerClus = GetFatSecPerClus();
    uint32_t physicalSectorNumber;  // absolute (phyiscal) sector number
    uint32_t dataRegionFirstSector = GetFatRsvdSecCnt() + (GetFatNumFATs() * GetFatFATSz32()); // Data Region First Sector 
    uint32_t cluster;

    //get FAT index for file's first cluster
    cluster =  fileSector[entry+21];
    cluster <<= 8;
    cluster |= fileSector[entry+20];
    cluster <<= 8;
    cluster |= fileSector[entry+27];
    cluster <<= 8;
    cluster |= fileSector[entry+26];
    
    // read in contents of file starting at relative sector 0 in 'cluster' and print contents to the screen.
    do
    {
        print_str("\n\n\r");   
        for(int clusterSectorNumber = 0; clusterSectorNumber < secPerClus; clusterSectorNumber++) 
        {
            physicalSectorNumber = clusterSectorNumber + dataRegionFirstSector + ( (cluster - 2) * secPerClus );

            fat_ReadSingleSector( bytsPerSec * physicalSectorNumber, fileSector);
            for(int k = 0; k < bytsPerSec; k++)  
            {
                if (fileSector[k] == '\n') print_str("\n\r");
                else if(fileSector[k] == 0);
                else USART_Transmit(fileSector[k]);
            }
        }
    } while( ( (cluster = pvt_GetNextCluster(cluster)) != 0x0FFFFFFF ) );
}