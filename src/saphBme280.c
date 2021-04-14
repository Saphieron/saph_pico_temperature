#include "saphBme280.h"
#include "i2c_handler.h"

#include <stdbool.h>

// Register Addresses of the BME280
#define REG_RESET_ADDR 0xE0
#define REG_HUMIDITY_CTRL_ADDR 0xF2
#define REG_STATUS_ADDR 0xF3
#define REG_MEASURE_CONTROL_ADDR 0xF4
#define REG_CONFIG_ADDR 0xF5
#define REG_DEVICE_ID_ADDR 0xD0
#define REG_PRESSURE_START_ADDR 0xF7

// Magic values used by/in the BME280
#define REG_RESET_VALUE 0xB6
#define STANDBY_TIME_POS 5
#define IIR_FILTER_COEFFICIENT_POS 2
#define MEASUREMENT_DATA_AMOUNT 8

// Local helpers to avoid magic numbers
#define BITMASK_LOWEST_FOUR 0x0F
#define BITMASK_LOWEST_THREE 0x07
#define BITMASK_LOWEST_TWO 0x03

#define TEMP_OVERSAMPLING_POS 5
#define PRESSURE_OVERSAMPLING_POS 2

// Helper function declarations
static int32_t getErrorCode(int32_t commResult, bool wasWriting);

static int32_t writeRegisterValueToDevice(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize);

static int32_t
readRegisterFromDevice(saphBmeDevice_t* device, uint8_t regAddress, uint8_t* readingBuffer, uint32_t readAmount);

static uint32_t getMeasurement20BitFromBuffer(const uint8_t* buffer);

static uint32_t getMeasurement16itFromBuffer(const uint8_t* buffer);

static int32_t readTrimmingValues(saphBmeDevice_t* device, uint8_t* buffer);

static inline void setTemperatureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer);

static inline void setPressureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer);

saphBmeDevice_t saphBme280_init(uint8_t address) {
    saphBmeDevice_t newDevice;
    newDevice.address = address;
    return newDevice;
}

int32_t saphBme280_getId(saphBmeDevice_t* device) {
    uint8_t idRegister = REG_DEVICE_ID_ADDR;
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
saphBme280_prepareConfigurationReg(saphBmeDevice_t* device, uint8_t standbyTime,
                                   uint8_t iirFilterCoefficient) {
//    standbyTime &= BITMASK_LOWEST_THREE;
    iirFilterCoefficient &= BITMASK_LOWEST_THREE;
    device->registerConfig =
            standbyTime << STANDBY_TIME_POS | iirFilterCoefficient << IIR_FILTER_COEFFICIENT_POS;
}

int32_t saphBme280_commitConfigReg(saphBmeDevice_t* device) {
    uint8_t buffer[] = {REG_CONFIG_ADDR, device->registerConfig};
    return writeRegisterValueToDevice(device, buffer, 2);
}

void saphBme280_prepareCtrlHumidityReg(saphBmeDevice_t* device, uint8_t humidityOversampling) {
    device->registerCtrlHumidity = humidityOversampling & BITMASK_LOWEST_THREE;
}

int32_t saphBme280_commitCtrlHumidity(saphBmeDevice_t* device) {
    uint8_t buffer[] = {REG_HUMIDITY_CTRL_ADDR, device->registerCtrlHumidity};
    return writeRegisterValueToDevice(device, buffer, 2);
}

int32_t saphBme280_status(saphBmeDevice_t* device, uint8_t* buffer) {
    return readRegisterFromDevice(device, REG_STATUS_ADDR, buffer, 1);
}

int32_t
saphBme280_getRawMeasurement(saphBmeDevice_t* device, saphBmeMeasurements_t* result) {
    uint8_t receiveBuffer[MEASUREMENT_DATA_AMOUNT];
    int32_t commResult = readRegisterFromDevice(device, REG_PRESSURE_START_ADDR, (uint8_t*) receiveBuffer,
                                                MEASUREMENT_DATA_AMOUNT);
    if (commResult != SAPH_BME280_NO_ERROR) {
        return commResult;
    }

    result->pressure = getMeasurement20BitFromBuffer(receiveBuffer);
    result->temperature = getMeasurement20BitFromBuffer(receiveBuffer + 3);
    result->humidity = getMeasurement16itFromBuffer(receiveBuffer + 6);
    return SAPH_BME280_NO_ERROR;
}

int32_t saphBme280_getPressure(saphBmeDevice_t* device, uint32_t* resultBuffer) {
    uint32_t readAmount = 3;
    int32_t commResult = readRegisterFromDevice(device, REG_PRESSURE_START_ADDR, (uint8_t*) resultBuffer, readAmount);
    if (commResult != SAPH_BME280_NO_ERROR) {
        return commResult;
    }
    *resultBuffer = getMeasurement20BitFromBuffer((uint8_t*) resultBuffer);
    return SAPH_BME280_NO_ERROR;
}

int32_t saphBme280_readTrimmingValues(saphBmeDevice_t* device) {

    uint8_t buffer[25 + 1 + 6];
    int32_t errorCode = readTrimmingValues(device, buffer);
    if(errorCode != SAPH_BME280_NO_ERROR){
        return errorCode;
    }
    setTemperatureTrimmingValues(device, buffer);
    setPressureTrimmingValues(device, buffer);

    device->trimmingValues.dig_H1 = buffer[25];
    device->trimmingValues.dig_H2 = (buffer[27] << 8) + buffer[26];
    device->trimmingValues.dig_H3 = buffer[28];
    device->trimmingValues.dig_H4 = (buffer[29] << 4) + (buffer[30] & BITMASK_LOWEST_FOUR);
    device->trimmingValues.dig_H5 = (buffer[31] << 4) + (buffer[30] >> 4);

    return SAPH_BME280_NO_ERROR;
}

// ###############################################
// Helper Functions
// ###############################################

static int32_t readTrimmingValues(saphBmeDevice_t* device, uint8_t* buffer) {
    uint8_t startingAddressFirst = 0x88;
    uint8_t startingAddressSecond = 0xA1;
    uint8_t startingAddressThird = 0xE1;
    int32_t errorCode = readRegisterFromDevice(device, startingAddressFirst, buffer, 25);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        return errorCode;
    }

    errorCode = readRegisterFromDevice(device, startingAddressSecond, buffer + 25, 1);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        return errorCode;
    }
    errorCode = readRegisterFromDevice(device, startingAddressThird, buffer + 25 + 1, 6);
    return errorCode;
}

static inline void setTemperatureTrimmingValues(saphBmeDevice_t* device, const uint8_t* buffer) {
    uint8_t i = 0;
    device->trimmingValues.dig_T1 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_T2 = (buffer[i + 1] << 8) + buffer[i];
    i += 2;
    device->trimmingValues.dig_T3 = (buffer[i + 1] << 8) + buffer[i];
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

static int32_t writeRegisterValueToDevice(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize) {
    int32_t commResult = i2c_handler_write(device->address, buffer, bufferSize);
    if (commResult != bufferSize) {
        return getErrorCode(commResult, true);
    }
    return SAPH_BME280_NO_ERROR;
}

static int32_t
readRegisterFromDevice(saphBmeDevice_t* device, uint8_t regAddress, uint8_t* readingBuffer, uint32_t readAmount) {
    int32_t commResult = 0;
    if ((commResult = writeRegisterValueToDevice(device, &regAddress, 1)) != SAPH_BME280_NO_ERROR) {
        return commResult;
    }

    commResult = i2c_handler_read(device->address, readingBuffer, readAmount);
    if (commResult != readAmount) {
        return getErrorCode(commResult, false);
    }

    return SAPH_BME280_NO_ERROR;
}

static uint32_t getMeasurement20BitFromBuffer(const uint8_t* buffer) {
    return (buffer[0] << 11) + (buffer[1] << 3) + (buffer[2] & BITMASK_LOWEST_THREE);
}

static uint32_t getMeasurement16itFromBuffer(const uint8_t* buffer) {
    return (buffer[0] << 8) + (buffer[1]);
}