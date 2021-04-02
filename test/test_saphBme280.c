#include "unity.h"

#include "saphBme280.h"

#include "mock_i2c_handler.h"

void setUp(void) {
}

void tearDown(void) {
}

/* *
 * TODO: Check notes, cross out what you have
 * Notes:
 *  - Sensor has a mode, allow it to be set. sleep mode, normal mode, forced mode
 *  - This might mean i need to allow for a i2cwrite to finish on a repeated start or no stop
 *
 * Steps for operating it:
 *  - The sensor 'boots' in sleep mode. Set mode[1:0] to 11 i.e. normal so that it starts? or 01 for a single measurement
 *  - After that:
 *  - Writing to a bme register is: slave address, the register address, and the register value. The content i need to write is therefore only the register addr and its value before sending a final stop.
 *  - Reading is: Write register, send stop, then read n bytes until you have all subsequent registers read. send stop to finish
 *
 * Configurations:
 *      -
 *
 * */

#define NO_ERROR 0
#define ERROR_PLATFORM_GENERIC -2
#define COMM_ERROR_WRITE_AMOUNT -11
#define COMM_ERROR_READ_AMOUNT -12

saphBmeDevice_t helper_createBmeDevice(void){
    uint8_t deviceAddr = 0xF7;
    saphBmeDevice_t bmeDevice = {deviceAddr};
    return bmeDevice;
}

void test_saphBme280_initialises(void) {
    // Might add some more config settings to it
    uint8_t deviceAddr = 0xAB;
    saphBmeDevice_t bmeDevice = saphBme280_init(deviceAddr);
    TEST_ASSERT_EQUAL_UINT8(deviceAddr, bmeDevice.address);
}

// #############################################
// # Test group _getId
// #############################################

void test_saphBme280_getId(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t expectedDeviceId = 0x58; //according to the BME280 datasheet
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    uint8_t response = expectedDeviceId;
    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ReturnArrayThruPtr_buffer(&response, 1);

    int32_t actualId = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedDeviceId, actualId);
}

void test_saphBme280_getId_returnsErrorOnFailedWriteAmount(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int16_t expectedError = COMM_ERROR_WRITE_AMOUNT;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 0);
    int16_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedError, errorCode);
}

void test_saphBme280_getId_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int16_t expectedError = ERROR_PLATFORM_GENERIC; // Supposedly the generic error code for the pico
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, expectedError);
    int16_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedError, errorCode);
}

void test_saphBme280_getId_returnsErrorOnFailedReadAmount(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int16_t expectedError = COMM_ERROR_READ_AMOUNT;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(0);
    int16_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedError, errorCode);
}

void test_saphBme280_getId_returnsI2cErrorCodeIfGenericHardwareErrorOnRead(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int16_t expectedError = ERROR_PLATFORM_GENERIC;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(expectedError);
    int16_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedError, errorCode);
}

// #############################################
// # Test group _resetDevice
// #############################################

void test_saphBme280_reset_deviceIsReset(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t resetRegisterAddr = 0xE0;
    uint8_t registerValueForResetting = 0xB6;
    uint8_t buffer[2] = {resetRegisterAddr, registerValueForResetting};
    uint32_t amount = 2;
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, buffer, amount, amount, amount);
    int32_t result = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(SAPH_BME280_NO_ERROR, result);
}

void test_saphBme280_reset_returnsErrorOnFailedWriteAmount(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = COMM_ERROR_WRITE_AMOUNT;
    i2c_handler_write_ExpectAnyArgsAndReturn(0);
    int32_t errorCode = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_reset_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    i2c_handler_write_ExpectAnyArgsAndReturn(ERROR_PLATFORM_GENERIC);
    int32_t errorCode = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _setSensorMode
// #############################################

void test_saphBme280_setSensorMode(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    TEST_FAIL_MESSAGE("Not implemented yet");
}