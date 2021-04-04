#include "saphBme280.h"
#include "i2c_handler.h"

#include <stdbool.h>

// Register Addresses of the BME280
#define REG_RESET_ADDR 0xE0
#define REG_MEASURE_CONTROL_ADDR 0xF4
#define REG_CONFIG_ADDR 0xF5

// Magic values used by/in the BME280
#define REG_RESET_VALUE 0xB6
#define STANDBY_TIME_POS 5
#define IIR_FILTER_COEFFICIENT_POS 1

// Local helpers to avoid magic numbers
#define BITMASK_LOWEST_THREE 7
#define BITMASK_LOWEST_TWO 3
#define TEMP_OVERSAMPLING_POS 5
#define PRESSURE_OVERSAMPLING_POS 2

static int32_t getErrorCode(int32_t commResult, bool wasWriting);

static int32_t writeRegisterValueToDevice(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize);

saphBmeDevice_t saphBme280_init(uint8_t address) {
    saphBmeDevice_t newDevice;
    newDevice.address = address;
    return newDevice;
}

int32_t saphBme280_getId(saphBmeDevice_t* device) {
    uint8_t idRegister = 0xD0;
    int32_t commResult = 0;
    if ((commResult = writeRegisterValueToDevice(device, &idRegister, 1)) != SAPH_BME280_NO_ERROR) {
        return commResult;
    }

    uint8_t id = 0;
    commResult = i2c_handler_read(device->address, &id, 1);
    if (commResult != 1) {
        return getErrorCode(commResult, false);
    }
    int32_t result = 0 + id;
    return result;
}

int32_t saphBme280_resetDevice(saphBmeDevice_t* device) {
    uint8_t buffer[2] = {REG_RESET_ADDR, REG_RESET_VALUE};
    return writeRegisterValueToDevice(device, buffer, 2);
}

static int32_t getErrorCode(int32_t commResult, bool wasWriting) {
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


void
saphBme280_prepareMeasureControlReg(saphBmeDevice_t* device, uint8_t tempOversampling, uint8_t pressureOversampling,
                                    uint8_t sensorMode) {
//    tempOversampling &= BITMASK_LOWEST_THREE;
    pressureOversampling &= BITMASK_LOWEST_THREE;
    sensorMode &= BITMASK_LOWEST_TWO;
    device->registerMeasureControl =
            tempOversampling << TEMP_OVERSAMPLING_POS | pressureOversampling << PRESSURE_OVERSAMPLING_POS | sensorMode;
}


int32_t saphBme280_commitMeasureControlReg(saphBmeDevice_t* device) {
    uint8_t buffer[2] = {REG_MEASURE_CONTROL_ADDR, device->registerMeasureControl};
    return writeRegisterValueToDevice(device, buffer, 2);
}


void
saphBme280_prepareConfigurationReg(saphBmeDevice_t* device, uint8_t standbyTimeInNormalMode,
                                   uint8_t iirFilterCoefficient) {
//    standbyTimeInNormalMode &= BITMASK_LOWEST_THREE;
    iirFilterCoefficient &= BITMASK_LOWEST_THREE;
    device->registerConfig =
            standbyTimeInNormalMode << STANDBY_TIME_POS | iirFilterCoefficient << IIR_FILTER_COEFFICIENT_POS;
}

int32_t saphBme280_commitConfigReg(saphBmeDevice_t* device) {
    uint8_t buffer[] = {REG_CONFIG_ADDR, device->registerConfig};
    return writeRegisterValueToDevice(device, buffer, 2);
}

#include <stdio.h>

static int32_t writeRegisterValueToDevice(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize) {
    int32_t commResult = i2c_handler_write(device->address, buffer, bufferSize);
    if (commResult != bufferSize) {
        return getErrorCode(commResult, true);
    }
    return SAPH_BME280_NO_ERROR;
}
