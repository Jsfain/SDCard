#ifndef FATTOSD_H
#define FATTOSD_H

#include <avr/io.h>

void fat_ReadSingleSector( uint32_t address, uint8_t * arr );

#endif //FATTOSD_H