#include <stdint.h>
#include <avr/io.h>
#include "../includes/usart.h"
#include "../includes/sd_spi.h"
#include "../includes/sd_misc.h"
#include "../includes/spi.h"
#include "../includes/fat32.h"
#include "../includes/prints.h"


fat32BPB fat32_getBPB(uint8_t *sectorZero)
{
    //there must be a volume sector 0 read into an array which is then passed as the argument here.
    //this function will parse the array values into their necessary parts of the BPB and store these
    //in the member struct variables.

    fat32BPB bpb;

    //check boot signature is present
    if((sectorZero[510] == 0x55) && (sectorZero[511]==0xAA))
    {
        //BS_jmpBoot - first 3 bytes of boot sector are jump instruction to boot code
        for(int i=0;i<3;i++) {bpb.BS_jmpBoot[i] = sectorZero[i];}

        //BS_OEMName
        for(int i=3;i<3+8;i++) {bpb.BS_OEMName[i-3] = sectorZero[i];}
        bpb.BS_OEMName[8] = '\0';

        //BPB_BytsPerSec
        bpb.BPB_BytsPerSec = sectorZero[12];
        bpb.BPB_BytsPerSec <<= 8;
        bpb.BPB_BytsPerSec |= sectorZero[11];

        //BPB_SecPeClus
        bpb.BPB_SecPerClus = sectorZero[13];

        //BPB_RsvdSecCnt
        bpb.BPB_RsvdSecCnt = sectorZero[15];
        bpb.BPB_RsvdSecCnt <<= 8;
        bpb.BPB_RsvdSecCnt|= sectorZero[14];

        //BPB_NumFATs
        bpb.BPB_NumFATs = sectorZero[16];

        //BPB_RootEntCnt
        bpb.BPB_RootEntCnt = sectorZero[18];
        bpb.BPB_RootEntCnt <<= 8;
        bpb.BPB_RootEntCnt |= sectorZero[17];

        //BPB_TotSec16
        bpb.BPB_TotSec16 = sectorZero[20];
        bpb.BPB_TotSec16 <<= 8;
        bpb.BPB_TotSec16 |= sectorZero[19];

        //BPB_Media
        bpb.BPB_Media = sectorZero[21];

        //BPB_FATSz16
        bpb.BPB_FATSz16 = sectorZero[23];
        bpb.BPB_FATSz16 <<= 8;
        bpb.BPB_FATSz16 |= sectorZero[22];

        //BPB_SecPerTrk
        bpb.BPB_SecPerTrk = sectorZero[25];
        bpb.BPB_SecPerTrk <<= 8;
        bpb.BPB_SecPerTrk |= sectorZero[24];

        //BPB_NumHeads
        bpb.BPB_NumHeads = sectorZero[27];
        bpb.BPB_NumHeads <<= 8;
        bpb.BPB_NumHeads |= sectorZero[26];

        //BPB_HiddSec
        bpb.BPB_HiddSec = sectorZero[31];
        bpb.BPB_HiddSec <<= 8;
        bpb.BPB_HiddSec |= sectorZero[30];
        bpb.BPB_HiddSec <<= 8;
        bpb.BPB_HiddSec |= sectorZero[29];
        bpb.BPB_HiddSec <<= 8;
        bpb.BPB_HiddSec |= sectorZero[28];
        
        //BPB_TotSec32
        bpb.BPB_TotSec32 = sectorZero[35];
        bpb.BPB_TotSec32 <<= 8;
        bpb.BPB_TotSec32 |= sectorZero[34];
        bpb.BPB_TotSec32 <<= 8;
        bpb.BPB_TotSec32 |= sectorZero[33];
        bpb.BPB_TotSec32 <<= 8;
        bpb.BPB_TotSec32 |= sectorZero[32];

        // ***** Extended BPB Structure for FAT32 volumes ***** 

        //BPB_FATSz32
        bpb.BPB_FATSz32 = sectorZero[39];
        bpb.BPB_FATSz32 <<= 8;
        bpb.BPB_FATSz32 |= sectorZero[38];
        bpb.BPB_FATSz32 <<= 8;
        bpb.BPB_FATSz32 |= sectorZero[37];
        bpb.BPB_FATSz32 <<= 8;
        bpb.BPB_FATSz32 |= sectorZero[36];

        //BPB_ExtFlags
        bpb.BPB_ExtFlags = sectorZero[41];
        bpb.BPB_ExtFlags <<= 8;
        bpb.BPB_ExtFlags |= sectorZero[40];

        //BPB_FSVer
        bpb.BPB_FSVer = sectorZero[43];
        bpb.BPB_FSVer <<= 8;
        bpb.BPB_FSVer |= sectorZero[42];

        //BPB_RootClus
        bpb.BPB_RootClus = sectorZero[47];
        bpb.BPB_RootClus <<= 8;
        bpb.BPB_RootClus |= sectorZero[46];
        bpb.BPB_RootClus <<= 8;
        bpb.BPB_RootClus |= sectorZero[45];
        bpb.BPB_RootClus <<= 8;
        bpb.BPB_RootClus |= sectorZero[44];

        //BPB_FSInfo
        bpb.BPB_FSInfo = sectorZero[49];
        bpb.BPB_FSInfo <<= 8;
        bpb.BPB_FSInfo |= sectorZero[48];
    
        //BPB_BkBootSec
        bpb.BPB_BkBootSec  = sectorZero[51];
        bpb.BPB_BkBootSec <<= 8;
        bpb.BPB_BkBootSec |= sectorZero[50];

        //BPB_Reserved
        for(int i=52;i<52+12;i++) {bpb.BPB_Reserved[i-52] = sectorZero[i];}

        //BS_DrvNum
        bpb.BS_DrvNum = sectorZero[64];

        //BS_Reserved1
        bpb.BS_Reserved1 = sectorZero[65];

        //BS_BootSig
        bpb.BS_BootSig = sectorZero[66];

        //BS_VolID
        bpb.BS_VolID = sectorZero[70];
        bpb.BS_VolID <<= 8;
        bpb.BS_VolID = sectorZero[69];
        bpb.BS_VolID <<= 8;
        bpb.BS_VolID = sectorZero[68];
        bpb.BS_VolID <<= 8;
        bpb.BS_VolID = sectorZero[67];
        
        //BS_VolLab
        for(int i=71;i<71+11;i++) {bpb.BS_VolLab[i-71] = sectorZero[i];}
        bpb.BS_VolLab[11] = '\0';

        //BS_FilSysType
        for(int i=82;i<82+8;i++) {bpb.BS_FilSysType[i-82] = sectorZero[i];}
        bpb.BS_FilSysType[8]='\0';

        //BootStrap - not a FAT32 field.
        for(int i=90;i<90+420;i++) {bpb.BootStrap[i-90] = sectorZero[i];}


        //Signature_word
        bpb.Signature_word  = sectorZero[511];
        bpb.Signature_word <<= 8;
        bpb.Signature_word |= sectorZero[510];
    }
    else
    {
        bpb.ERROR = INVALID_BOOT_SIGNATURE;
        print_str("\n\n\r>>Invalid boot record signature.");
        print_str("\n\r>>Returning with error code"); print_dec(bpb.ERROR);
        print_str("\n\r>>boot sector NOT LOADED.\n\r");
        return bpb;
    }

    return bpb;
}



void fat32_printBPB(fat32BPB bpb)
{
    print_str("\n\n\rBS_jmpBoot = 0x"); 
    print_hex(bpb.BS_jmpBoot[0]);
    print_hex(bpb.BS_jmpBoot[1]);
    print_hex(bpb.BS_jmpBoot[2]);

    print_str("\n\rBS_OEMName = ");
    print_str(bpb.BS_OEMName);
    
    print_str("\n\rBPB_BytsPerSec = ");
    print_dec(bpb.BPB_BytsPerSec);

    print_str("\n\rBPB_SecPerClus = ");
    print_dec(bpb.BPB_SecPerClus);

    print_str("\n\rBPB_RsvdSecCnt = ");
    print_dec(bpb.BPB_RsvdSecCnt);

    print_str("\n\rBPB_NumFATs = ");
    print_dec(bpb.BPB_NumFATs);

    print_str("\n\rBPB_RootEntCnt = ");
    print_dec(bpb.BPB_RootEntCnt);
    
    print_str("\n\rBPB_TotSec16 = ");
    print_dec(bpb.BPB_TotSec16);
    
    print_str("\n\rBPB_Media = 0x");
    print_hex(bpb.BPB_Media);
    
    print_str("\n\rBPB_FATSz16 = ");
    print_dec(bpb.BPB_FATSz16);
    
    print_str("\n\rBPB_SecPerTrk = ");
    print_dec(bpb.BPB_SecPerTrk);
    
    print_str("\n\rBPB_NumHeads = ");
    print_dec(bpb.BPB_NumHeads);
    
    print_str("\n\rBPB_HiddSec = ");
    print_dec(bpb.BPB_HiddSec);
    
    print_str("\n\rBPB_TotSec32 = ");
    print_dec(bpb.BPB_TotSec32);

    //Extended BPB Structure for FAT32 volumes
    print_str("\n\rBPB_FATSz32 = ");
    print_dec(bpb.BPB_FATSz32);

    print_str("\n\rBPB_ExtFlags = 0x");
    print_hex(bpb.BPB_ExtFlags);

    print_str("\n\rBPB_FSVer = ");
    print_dec(bpb.BPB_FSVer);

    print_str("\n\rBPB_RootClus = ");
    print_dec(bpb.BPB_RootClus);

    print_str("\n\rBPB_FSInfo = ");
    print_dec(bpb.BPB_FSInfo);

    print_str("\n\rBPB_BkBootSec = ");
    print_dec(bpb.BPB_BkBootSec);

    print_str("\n\rBS_DrvNum = ");
    print_dec(bpb.BS_DrvNum);

    print_str("\n\rBS_BootSig = 0x");
    print_hex(bpb.BS_BootSig);

    print_str("\n\rBS_VolID = 0x");
    print_hex(bpb.BS_VolID);

    print_str("\n\rBS_VolLab = ");
    print_str(bpb.BS_VolLab);

    print_str("\n\rBS_FilSysType = ");
    print_str(bpb.BS_FilSysType);

    print_str("\n\rBootStrap\n\r");
    for(int i = 0; i <420; i++)
    {
        if((bpb.BootStrap[i]<128) && (bpb.BootStrap[i]>31))
            USART_Transmit(bpb.BootStrap[i]);
        else
            print_str(" ");
    }
    //for(int i=0;i<420;i++){print_str("%c\t",bpb.FAT32_BootCode[i]);}
    print_str("\n\rSignature_word = 0x");
    print_hex(bpb.Signature_word);
}



fatFSINFO getFSINFO(uint8_t *sector)
{
    fatFSINFO fsinfo;

    
    //FSI_LeadSig
    fsinfo.FSI_LeadSig = sector[3];
    fsinfo.FSI_LeadSig <<= 8;
    fsinfo.FSI_LeadSig |= sector[2];
    fsinfo.FSI_LeadSig <<= 8;
    fsinfo.FSI_LeadSig |= sector[1];
    fsinfo.FSI_LeadSig <<= 8;
    fsinfo.FSI_LeadSig |= sector[0];

    if(fsinfo.FSI_LeadSig != 0x41615252)
    {
        print_str("\n\rInvalid FSI_LeadSig read. Returning without storing FSInfo fields.\n\r");
        fsinfo.ERROR = 1;
        return fsinfo;
    }

    //FSI_StructSig
    fsinfo.FSI_StructSig = sector[487];
    fsinfo.FSI_StructSig <<= 8;
    fsinfo.FSI_StructSig |= sector[486];
    fsinfo.FSI_StructSig <<= 8;
    fsinfo.FSI_StructSig |= sector[485];
    fsinfo.FSI_StructSig <<= 8;
    fsinfo.FSI_StructSig |= sector[484];

    if(fsinfo.FSI_StructSig != 0x61417272)
    {
        print_str("\n\rInvalid FSI_StructSig read. Returning without storing rest of FSInfo fields.\n\r");
        fsinfo.ERROR = 1;
        return fsinfo;
    }

    //FSI_Free_Count
    fsinfo.FSI_Free_Count = sector[491];
    fsinfo.FSI_Free_Count <<= 8;
    fsinfo.FSI_Free_Count |= sector[490];
    fsinfo.FSI_Free_Count <<= 8;
    fsinfo.FSI_Free_Count |= sector[489];
    fsinfo.FSI_Free_Count <<= 8;
    fsinfo.FSI_Free_Count |= sector[488];

    //FSI_Nxt_Free
    fsinfo.FSI_Nxt_Free = sector[495];
    fsinfo.FSI_Nxt_Free <<= 8;
    fsinfo.FSI_Nxt_Free |= sector[494];
    fsinfo.FSI_Nxt_Free <<= 8;
    fsinfo.FSI_Nxt_Free |= sector[493];
    fsinfo.FSI_Nxt_Free <<= 8;
    fsinfo.FSI_Nxt_Free |= sector[492];


    //FSI_TrailSig
    fsinfo.FSI_TrailSig = sector[511];
    fsinfo.FSI_TrailSig <<= 8;
    fsinfo.FSI_TrailSig |= sector[510];
    fsinfo.FSI_TrailSig <<= 8;
    fsinfo.FSI_TrailSig |= sector[509];
    fsinfo.FSI_TrailSig <<= 8;
    fsinfo.FSI_TrailSig |= sector[508];


    return fsinfo;
}



void print_fatFSINFO_Fields(fatFSINFO fsinfo)
{
    print_str("\n\n\rFSI_LeadSig = 0x");
    print_hex(fsinfo.FSI_LeadSig);

    print_str("\n\rFSI_StructSig = 0x");
    print_hex(fsinfo.FSI_StructSig);

    print_str("\n\rFSI_Free_Count (HEX) = 0x");
    print_hex(fsinfo.FSI_Free_Count);

    print_str("\n\rFSI_Free_Count (DECIMAL) = ");
    print_dec(fsinfo.FSI_Free_Count);

    print_str("\n\rFSI_Nxt_Free = ");
    print_dec(fsinfo.FSI_Nxt_Free);

    print_str("\n\rFSI_TrailSig = 0x");
    print_hex(fsinfo.FSI_TrailSig);
}
