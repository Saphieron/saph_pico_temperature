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
#define MEASUREMENT_DATA_AMOUNT 9

// Local helpers to avoid magic numbers
#define BITMASK_LOWEST_THREE 7
#define BITMASK_LOWEST_TWO 3
#define TEMP_OVERSAMPLING_POS 5
#define PRESSURE_OVERSAMPLING_POS 2

// Helper function declarations
static int32_t getErrorCode(int32_t commResult, bool wasWriting);
static int32_t writeRegisterValueToDevice(saphBmeDevice_t* device, uint8_t* buffer, uint32_t bufferSize);
static int32_t
readRegisterValueFromDevice(saphBmeDevice_t* device, uint8_t regAddress, uint8_t* readingBuffer, uint32_t readAmount);
static uint32_t getMeasurementFromBuffer(const uint8_t* buffer);

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
    return writeRegisterValueToDevice(device,buffer, 2);
}

int32_t saphBme280_status(saphBmeDevice_t* device, uint8_t* buffer) {
    return readRegisterValueFromDevice(device, REG_STATUS_ADDR, buffer, 1);
}

int32_t
saphBme280_getRawMeasurement(saphBmeDevice_t* device, uint32_t* pressure, uint32_t* temp, uint32_t* humidity) {
    uint8_t receiveBuffer[MEASUREMENT_DATA_AMOUNT];
    int32_t commResult = readRegisterValueFromDevice(device, REG_PRESSURE_START_ADDR, (uint8_t*) receiveBuffer,
                                                     MEASUREMENT_DATA_AMOUNT);
    if (commResult != SAPH_BME280_NO_ERROR) {
        return commResult;
    }
    *pressure = getMeasurementFromBuffer(receiveBuffer);
    *temp = getMeasurementFromBuffer(receiveBuffer + 3);
    *humidity = getMeasurementFromBuffer(receiveBuffer + 6);
    return SAPH_BME280_NO_ERROR;
}

int32_t saphBme280_getPressure(saphBmeDevice_t* device, uint32_t* resultBuffer) {
    uint32_t readAmount = 3;
    int32_t commResult = readRegisterValueFromDevice(device, REG_PRESSURE_START_ADDR, (uint8_t*) resultBuffer,
                                                     readAmount);
    if (commResult != SAPH_BME280_NO_ERROR) {
        return commResult;
    }

    uint8_t* helper = (uint8_t*) resultBuffer;
    uint32_t pressureFixed = helper[0] << 12 | helper[1] << 4 | (helper[2] & BITMASK_LOWEST_THREE);
    *resultBuffer = pressureFixed;
    return SAPH_BME280_NO_ERROR;
}

// ###############################################
// Helper Functions
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
readRegisterValueFromDevice(saphBmeDevice_t* device, uint8_t regAddress, uint8_t* readingBuffer, uint32_t readAmount) {
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

static uint32_t getMeasurementFromBuffer(const uint8_t* buffer) {
    return (buffer[0] << 11) + (buffer[1] << 3) + (buffer[2] & BITMASK_LOWEST_THREE);
}