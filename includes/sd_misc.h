#ifndef SDD1_H
#define SDD1_H


// Calculate and retrunt the memory capacity of the SD Card in Bytes.
uint32_t sd_getMemoryCapacity();


// ********** data block functions **************

void print_sector(uint8_t *sector);  //only 512 byte sector supported.

uint32_t getSector();

typedef struct  DataSector { //A block is limited to 512 bytes in this implementation
    uint8_t R1;
    uint8_t ERROR;
    uint8_t byte[DATA_BLOCK_LEN];
    uint8_t CRC[2];
} DataSector;

DataSector sd_ReadSingleDataSector(uint32_t address);


#endif