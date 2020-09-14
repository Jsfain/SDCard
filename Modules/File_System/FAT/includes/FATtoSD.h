#ifndef FATTOSD_H
#define FATTOSD_H

#include <avr/io.h>


void fat_ReadSingleSector( uint32_t address, uint8_t * arr );
void fat_PrintSingleSector( uint8_t *sectorByteArray );

#endif //FATTOSD_H