#include <sched.h>

#ifndef SAPHBME280_H
#define SAPHBME280_H

#include <stdint.h>

typedef struct saphBmeDevice_t{
    uint8_t address;
} saphBmeDevice_t;


saphBmeDevice_t saphBme280_init(uint8_t address);

uint8_t saphBme280_getId(saphBmeDevice_t* device);

#endif // SAPHBME280_H
