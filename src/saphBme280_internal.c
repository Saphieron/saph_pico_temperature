#include "saphBme280_internal.h"
#include "saphBme280.h"
#include "i2c_handler.h"

#include <stdbool.h>

typedef struct tempResults_t {
    int32_t fineTemperature;
    int32_t temperature;
} tempResults_t;

// ###############################################
// Helper Function definitions
// ###############################################

static uint32_t getMeasurement20BitFromBuffer(const uint8_t* buffer);

static uint32_t getMeasurement16itFromBuffer(const uint8_t* buffer);

static int32_t readTrimmingValues(saphBmeDevice_t* device, uint8_t* buffer);

static inline void setTemperatureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer);

static inline void setPressureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer);

static inline void setHumidityTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer);

static tempResults_t compensateTemperature(saphBmeDevice_t* device, int32_t rawTemperature);

static uint32_t compensatePressure(saphBmeDevice_t* device, int32_t rawPressure, int32_t fineTemperature);

static uint32_t compensateHumidity(saphBmeDevice_t* device, int32_t rawHumidity, int32_t fineTemperature);

// ###############################################
// Implementations
// ###############################################

int32_t saphBme280_internal_writeToRegister(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize) {
    int32_t commResult = i2c_handler_write(device->address, buffer, bufferSize);
    if (commResult != bufferSize) {
        return saphBme280_internal_getErrorCode(commResult, true);
    }
    return SAPH_BME280_NO_ERROR;
}


int32_t saphBme280_internal_readFromRegister(saphBmeDevice_t* device, uint8_t regAddress, uint8_t* readingBuffer,
                                             uint32_t readAmount) {
    int32_t commResult = 0;
    if ((commResult = saphBme280_internal_writeToRegister(device, &regAddress, 1)) != SAPH_BME280_NO_ERROR) {
        return commResult;
    }

    commResult = i2c_handler_read(device->address, readingBuffer, readAmount);
    if (commResult != readAmount) {
        return saphBme280_internal_getErrorCode(commResult, false);
    }

    return SAPH_BME280_NO_ERROR;
}

#define BURST_READ_TRIM_FIRST 25
#define BURST_READ_TRIM_SECOND 1
#define BURST_READ_TRIM_THIRD 7

int32_t saphBme280_internal_readTrimmingValues(saphBmeDevice_t* device) {
    uint8_t buffer[BURST_READ_TRIM_FIRST + BURST_READ_TRIM_SECOND + BURST_READ_TRIM_THIRD];
    int32_t errorCode = readTrimmingValues(device, buffer);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        return errorCode;
    }
    setTemperatureTrimmingValues(device, buffer);
    setPressureTrimmingValues(device, buffer);
    setHumidityTrimmingValues(device, buffer);

    return SAPH_BME280_NO_ERROR;
}

#define MEASUREMENT_DATA_AMOUNT 8
#define REG_PRESSURE_START_ADDR 0xF7

int32_t saphBme280_internal_getRawMeasurement(saphBmeDevice_t* device, saphBmeRawMeasurements_t* result) {
    uint8_t receiveBuffer[MEASUREMENT_DATA_AMOUNT];
    int32_t commResult = saphBme280_internal_readFromRegister(device, REG_PRESSURE_START_ADDR, (uint8_t*) receiveBuffer,
                                                              MEASUREMENT_DATA_AMOUNT);
    if (commResult != SAPH_BME280_NO_ERROR) {
        return commResult;
    }

    result->pressure = getMeasurement20BitFromBuffer(receiveBuffer);
    result->temperature = getMeasurement20BitFromBuffer(receiveBuffer + 3);
    result->humidity = getMeasurement16itFromBuffer(receiveBuffer + 6);
    return SAPH_BME280_NO_ERROR;
}

saphBmeMeasurements_t
saphBme280_internal_compensateMeasurements(saphBmeDevice_t* device, saphBmeRawMeasurements_t* rawMeasurements) {
    saphBmeMeasurements_t result;
    tempResults_t temps = compensateTemperature(device, rawMeasurements->temperature);
    result.temperature = temps.temperature;
    result.pressure = compensatePressure(device, rawMeasurements->pressure, temps.fineTemperature);
    result.humidity = compensateHumidity(device, rawMeasurements->humidity, temps.fineTemperature);
    return result;
}

int32_t saphBme280_internal_getErrorCode(int32_t commResult, bool wasWriting) {
    if (wasWriting) {
        if (commResult >= 0) {
            return SAPH_BME280_COMM_ERROR_WRITE_AMOUNT;
        } else {
            return commResult;
        }
    } else {
        if (commResult >= 0) {
            return SAPH_BME280_COMM_ERROR_READ_AMOUNT;
        } else {
            return commResult;
        }
    }
}


// ###############################################
// Helper Functions
// ###############################################

static uint32_t getMeasurement20BitFromBuffer(const uint8_t* buffer) {
    return (buffer[0] << 12) + (buffer[1] << 4) + (buffer[2] >> 4);
}

static uint32_t getMeasurement16itFromBuffer(const uint8_t* buffer) {
    return (buffer[0] << 8) + (buffer[1]);
}


static int32_t readTrimmingValues(saphBmeDevice_t* device, uint8_t* buffer) {
    uint8_t startingAddressFirst = 0x88;
    uint8_t startingAddressSecond = 0xA1;
    uint8_t startingAddressThird = 0xE1;
    int32_t errorCode = saphBme280_internal_readFromRegister(device, startingAddressFirst, buffer,
                                                             BURST_READ_TRIM_FIRST);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        return errorCode;
    }

    errorCode = saphBme280_internal_readFromRegister(device, startingAddressSecond, buffer + BURST_READ_TRIM_FIRST,
                                                     BURST_READ_TRIM_SECOND);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        return errorCode;
    }
    errorCode = saphBme280_internal_readFromRegister(device, startingAddressThird,
                                                     buffer + BURST_READ_TRIM_FIRST + BURST_READ_TRIM_SECOND,
                                                     BURST_READ_TRIM_THIRD);
    return errorCode;
}

static tempResults_t compensateTemperature(saphBmeDevice_t* device, int32_t rawTemperature) {
    saphBmeTrimmingValues_t* trims = &(device->trimmingValues);
    int32_t value1, value2;
    value1 = (rawTemperature >> 3) - (((int32_t) trims->dig_T1) << 1);
    value1 = (value1 * ((int32_t) trims->dig_T2)) >> 11;
    value2 = (rawTemperature >> 4) - ((int32_t) trims->dig_T1);
    value2 = (((value2 * value2) >> 12) * ((int32_t) trims->dig_T3)) >> 14;
    tempResults_t result;
    result.fineTemperature = value1 + value2;
    result.temperature = (result.fineTemperature * 5 + 128) >> 8;
    return result;
}

static uint32_t compensatePressure(saphBmeDevice_t* device, int32_t rawPressure, int32_t fineTemperature) {
    saphBmeTrimmingValues_t* trims = &(device->trimmingValues);
    int64_t value1, value2, pressure;
    value1 = ((int64_t) fineTemperature) - 128000;
    value2 = value1 * value1 * ((int64_t) trims->dig_P6);
    value2 = value2 + ((value1 * ((int64_t) trims->dig_P5)) << 17);
    value2 = value2 + (((int64_t) trims->dig_P4) << 35);
    value1 = ((value1 * value1 * (int64_t) trims->dig_P3) >> 8) + ((value1 * (int64_t) trims->dig_P2) << 12);
    value1 = (((((int64_t) 1) << 47) + value1) * ((int64_t) trims->dig_P1)) >> 33;
    if (value1 == 0) {
        return 0;
    }
    pressure = 1048576 - rawPressure;
    pressure = (((pressure << 31) - value2) * 3125) / value1;
    value1 = (((int64_t) trims->dig_P9) * (pressure >> 13) * (pressure > 13)) >> 25;
    value2 = (((int64_t) trims->dig_P8) * pressure) >> 19;
    pressure = ((pressure + value1 + value2) >> 8) + (((int64_t) trims->dig_P7) << 4);
    uint32_t result = (uint32_t) pressure;
    return result;
}

static uint32_t compensateHumidity(saphBmeDevice_t* device, int32_t rawHumidity, int32_t fineTemperature) {
    saphBmeTrimmingValues_t* trims = &(device->trimmingValues);
    int32_t variable; //I don't know what the datasheet was trying to tell me with their og name
    variable = (fineTemperature - ((int32_t) 76800));
    variable =
            ((((rawHumidity << 14) - (((int32_t) trims->dig_H4) << 20) -
               (((int32_t) trims->dig_H5) * variable)) + ((int32_t) 16348)) >> 15) *
            (((((((variable * ((int32_t) trims->dig_H6) >> 10) * (variable * ((int32_t) trims->dig_H3)) >> 11) +
                 ((int32_t) 32768)) >> 10) + ((int32_t) 2097152)) * ((int32_t) trims->dig_H2) + 8192) >> 14);
    variable = (variable - (((((variable >> 15) * (variable >> 15)) >> 7) * ((int32_t) trims->dig_H1)) >> 4));
    variable = (variable < 0 ? 0 : variable);
    return (uint32_t) (variable >> 12);
}

static inline void setTemperatureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer) {
    uint8_t i = 0;
    device->trimmingValues.dig_T1 = (buffer[i + 1] << 8) | buffer[i];
    i += 2;
    device->trimmingValues.dig_T2 = (buffer[i + 1] << 8) | buffer[i];
    i += 2;
    device->trimmingValues.dig_T3 = (buffer[i + 1] << 8) | buffer[i];
}

static inline void setPressureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer) {
    uint8_t i = 6;
    device->trimmingValues.dig_P1 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P2 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P3 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P4 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P5 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P6 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P7 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P8 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_P9 = (buffer[i + 1] << 8) + buffer[i];
}

#define BITMASK_LOWEST_FOUR 0x0F

static inline void setHumidityTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer) {
    device->trimmingValues.dig_H1 = buffer[25];
    device->trimmingValues.dig_H2 = (buffer[27] << 8) + buffer[26];
    device->trimmingValues.dig_H3 = buffer[28];
    device->trimmingValues.dig_H4 = (buffer[29] << 4) + (buffer[30] & BITMASK_LOWEST_FOUR);
    device->trimmingValues.dig_H5 = (buffer[31] << 4) + (buffer[30] >> 4);
    device->trimmingValues.dig_H6 = buffer[32];
}
