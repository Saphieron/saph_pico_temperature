#include "saphBme280.h"
#include "i2c_handler.h"

#include <stdbool.h>

static int32_t getErrorCode(int32_t commResult, bool wasWriting);

saphBmeDevice_t saphBme280_init(uint8_t address) {
    saphBmeDevice_t newDevice;
    newDevice.address = address;
    return newDevice;
}

int32_t saphBme280_getId(saphBmeDevice_t* device) {
    uint8_t idRegister = 0xD0;
    int32_t commResult = i2c_handler_write(device->address, &idRegister, 1);
    if (commResult != 1) {
        return getErrorCode(commResult, true);
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
    uint8_t buffer[2] = {SAPH_BME280_REG_RESET, SAPH_BME280_REG_RESET_VALUE};
    int32_t commResult =i2c_handler_write(device->address, buffer, 2);
    if (commResult != 2) {
        return getErrorCode(commResult, true);
    }
    return SAPH_BME280_NO_ERROR;
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