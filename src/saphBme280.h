#include <sched.h>

#ifndef SAPHBME280_H
#define SAPHBME280_H

#include <stdint.h>

typedef struct saphBmeDevice_t{
    uint8_t address;
} saphBmeDevice_t;

#define SAPH_BME280_NO_ERROR 0
#define SAPH_BME280_COMM_ERROR_WRITE_AMOUNT -11
#define SAPH_BME280_COMM_ERROR_READ_AMOUNT -12
#define SAPH_BME280_REG_RESET 0xE0
#define SAPH_BME280_REG_RESET_VALUE 0xB6

saphBmeDevice_t saphBme280_init(uint8_t address);

int32_t saphBme280_getId(saphBmeDevice_t* device);

int32_t saphBme280_resetDevice(saphBmeDevice_t* device);

#endif // SAPHBME280_H
