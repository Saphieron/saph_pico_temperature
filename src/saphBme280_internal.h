#ifndef SAPHBME280_INTERNAL_H
#define SAPHBME280_INTERNAL_H

#include <stdbool.h>
#include "saphBme280.h"

typedef struct saphBmeRawMeasurements_t {
    int32_t pressure;
    int32_t temperature;
    int32_t humidity;
} saphBmeRawMeasurements_t;

int32_t saphBme280_internal_readTrimmingValues(saphBmeDevice_t* device);

int32_t saphBme280_internal_getErrorCode(int32_t commResult, bool wasWriting);

int32_t saphBme280_internal_writeToRegister(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize);

int32_t saphBme280_internal_readFromRegister(saphBmeDevice_t* device, uint8_t regAddress, uint8_t* readingBuffer,
                                             uint32_t readAmount);

int32_t saphBme280_internal_getRawMeasurement(saphBmeDevice_t* device, saphBmeRawMeasurements_t* result);

saphBmeMeasurements_t
saphBme280_internal_compensateMeasurements(saphBmeDevice_t* device, saphBmeRawMeasurements_t* rawMeasurements);

#endif // SAPHBME280_INTERNAL_H
