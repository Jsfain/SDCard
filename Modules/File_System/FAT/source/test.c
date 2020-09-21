/******************************************************************************
 * Author: Joshua Fain
 * Date:   7/5/2020
 * 
 * Contians main()
 * File used to test out the sd card functions. Changes regularly
 * depending on what is being tested.
 * ***************************************************************************/

#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "../includes/usart.h"
#include "../includes/spi.h"
#include "../includes/prints.h"
#include "../includes/sd_spi_base.h"
#include "../includes/sd_spi_data_access.h"
#include "../includes/fat.h"


uint32_t enterBlockNumber();

//  *******   MAIN   ********  
int main(void)
{
    USART_Init();
    SPI_MasterInit();
    
    uint32_t initResponse;

    //attempt to initialize sd card.
    for(int i = 0; i<2; i++)
    {
        //print_str("\n\n\rSD Card Initialization Attempt: ");
        print_dec(i);
        initResponse = SD_InitializeSPImode();
        if(initResponse != OUT_OF_IDLE) // If response is anything other than 0 (OUT_OF_IDLE) then initialization failed.
        {    
            print_str("\n\rSD INIT FAILED RESPONSE = 0x");
            print_hex(initResponse);
            SD_PrintInitError(initResponse);
        }
        else
        {   
            //print_str("\n\rSD Card Successfully Initialized\n\r");
            break;
        }
    }

    if(initResponse==OUT_OF_IDLE) // initialization successful
    {          
        print_str("\n\r");
        
        //initialize current working directory to the root directory
        FatCurrentDirectory cwd = {"/","","/","", GetFatRootClus()};
        
        int quit = 0;   
        int len = 64;
        char str[len];
        char temp;

        int p;
        int s;

        char c[len];
        char a[len];
        
        uint8_t flag = 0;

        int noa = 0; //num of arguements
        int loc[len];
        int i = 0;
        int err = 0;
        
        do
        {
            flag = 0;
            err = 0;

            for(int k = 0; k < len; k++) str[k] = '\0';
            for(int k = 0; k < len; k++)   c[k] = '\0';
            for(int k = 0; k < len; k++)   a[k] = '\0';
            
            for(int k = 0; k < len; k++) loc[k] = 0;            
            noa = 0; //num of arguements
        
            print_str("\n\r");print_str(cwd.longParentPath);print_str(cwd.longName);print_str(" > ");
            temp = USART_Receive();
  
            i = 0;
            while(temp != '\r')
            {
                
                if(temp == 127)  // compensate for lack of backspace on MAC
                {
                    print_str("\b \b");
                    if(i > 0) i--;
                }

                else 
                { 
                    USART_Transmit(temp);
                    str[i] = temp;
                    i++;
                }

                temp = USART_Receive();
                if(i >= len) break;
            }


            for(p = 0; p < i; p++)
            {
                c[p] = str[p];
                if( c[p] == ' ' ) { c[p] = '\0'; break; }
                if( c[p] == '\0') break;
            }

            for(s = 0; s < i - p; s++)
            {
                a[s] = str[s+p+1];
                if( a[s] == '\0') break;
            }

            if (i < len) 
            {
                if(!strcmp(c,"cd"))
                {   
                    err = SetFatCurrentDirectory(&cwd, a);
                    PrintFatError(err);
                }
                
                else if (!strcmp(c,"ls"))
                {

                    loc[noa] = 0;
                    noa++;

                    for(int t = 0; t < len; t++)
                    {
                        if( a[t] == '\0') { break; }

                        if( a[t] ==  ' ')
                        {
                            a[t] = '\0';
                            loc[noa] = t+1;
                            noa++;
                        }
                    }

                    for(int t = 0; t < noa; t++)
                    {
                             if (strcmp(&a[loc[t]],"/LN") == 0  ) flag |= LONG_NAME;
                        else if (strcmp(&a[loc[t]],"/SN") == 0 ) flag |= SHORT_NAME;
                        else if (strcmp(&a[loc[t]],"/A")  == 0 ) flag |= ALL;
                        else if (strcmp(&a[loc[t]],"/H")  == 0 ) flag |= HIDDEN;
                        else if (strcmp(&a[loc[t]],"/C")  == 0 ) flag |= CREATION;
                        else if (strcmp(&a[loc[t]],"/LA") == 0 ) flag |= LAST_ACCESS;
                        else if (strcmp(&a[loc[t]],"/LM") == 0 ) flag |= LAST_MODIFIED;
                        else { print_str("\n\rInvalid Argument"); break; }
                        //print_str("\n\r flag    = \"0x");print_hex(flag);  print_str("\"");
                    }
                    
                    if((flag&SHORT_NAME) != SHORT_NAME) { flag |= LONG_NAME; } //long name is default
                    err = PrintFatCurrentDirectoryContents(&cwd, flag);
                    PrintFatError(err);
                }
                
                else if (!strcmp(c,"open")) 
                { 
                    err = PrintFatFileContents(&cwd,a);
                    PrintFatError(err);
                }
                else if (!strcmp(c,"cwd"))
                {
                    print_str("\n\rshortName = "); print_str(cwd.shortName);
                    print_str("\n\rshortParentPath = "); print_str(cwd.shortParentPath);
                    print_str("\n\rlongName = "); print_str(cwd.longName);
                    print_str("\n\rlongParentPath = "); print_str(cwd.longParentPath);
                    print_str("\n\rFATFirstCluster = "); print_dec(cwd.FATFirstCluster);
                }
                else if (c[0] == 'q') {  print_str("\n\rquit\n\r"); quit = 1; }
                
                else  print_str("\n\rInvalid command\n\r");
            }
        
            print_str("\n\r");
        
            for (int k = 0; k < 10; k++) UDR0; // ensure USART Data Register is cleared of any remaining garbage bytes.
        
        }while (quit == 0);      


        uint32_t start_sector;
        uint32_t start_address;
        uint32_t nos;
       
        uint8_t answer;
        do{
            do{
                print_str("\n\rEnter Start Sector\n\r");
                start_sector = enterBlockNumber();
                print_str("\n\rhow many sectors do you want to print?\n\r");
                nos = enterBlockNumber();
                print_str("\n\rYou want to print "); print_dec(nos);
                print_str(" sectors beginning at sector "); print_dec(start_sector);
                print_str("\n\ris this correct? (y/n)");
                answer = USART_Receive();
                USART_Transmit(answer);
                print_str("\n\r");
            }while(answer != 'y');


            start_address = DATA_BLOCK_LEN * start_sector;
            SD_PrintMultipleDataBlocks(start_address,nos);

            print_str("\n\r press 'q' to quit and any other key to go again: ");
            answer = USART_Receive();
            USART_Transmit(answer);

        }while(answer != 'q');
    }
    
    // Something to do after SD card testing has completed.
    while(1)
        USART_Transmit(USART_Receive());
    
    return 0;
}



uint32_t enterBlockNumber()
{
    uint8_t x;
    uint8_t c;
    uint32_t sector = 0;

    //print_str("Specify data block (data block = 512 bytes) and press enter\n\r");
    c = USART_Receive();
    
    while(c!='\r')
    {
        x = c - '0';
        sector = sector*10;
        sector += x;
        print_str("\r");
        print_dec(sector);
        c = USART_Receive();
        if(sector >= 4194304)
        {
            sector = 0;
            print_str("sector value is too large.\n\rEnter value < 4194304\n\r");
        }
    }
    return sector;
}
