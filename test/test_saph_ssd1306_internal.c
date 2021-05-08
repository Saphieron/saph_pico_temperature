#include "unity.h"

#include "saph_ssd1306_internal.h"
#include "test_saph_ssd1306_test_definitions.h"

#include "mock_i2c_handler.h"

/* *
 * Some general notes copied from the datasheet
 * You send commands.
 * A command starts with a control byte, A control byte mainly consists of Co and D/C# bits following by six “0” ‘s.
 *  - If the Co bit is set as logic “0”, the transmission of the following information will contain data bytes only.
 *  - The D/C# bit determines the next data byte is acted as a command or a data. If the D/C# bit is set to logic “0”,
 *      it defines the following data byte as a command. If the D/C# bit is set to logic “1”, it defines the
 *      following data byte as a data which will be stored at the GDDRAM. The GDDRAM column address pointer will
 *      be increased by one automatically after each data write.
 * */

// #############################################
// # Test group _sendCtrlCommand
// #############################################

saph_ssd1306_device_t helper_createTestDevice(void) {
    saph_ssd1306_device_t testDevice;
    testDevice.address = 0xAB;
    return testDevice;
}

void test_saph_ssd1306_internal_sendCtrlCommand(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    uint8_t dc_byte = 0; // A command with only data
    uint8_t paramBuffer[] = {0xFA, 0xFB};
    uint32_t bufferSize = 2;
    uint8_t expectedData[3] = {dc_byte, paramBuffer[0], paramBuffer[1]};
    i2c_handler_write_ExpectAndReturn(testDevice.address, expectedData, bufferSize + 1, NO_ERROR);
    int32_t errorCode = saph_ssd1306_internal_sendCtrlCommand(&testDevice, paramBuffer, bufferSize);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saph_ssd1306_internal_sendCtrlCommand_returnsErrorOnDeviceNullpointer(void) {
    saph_ssd1306_device_t* testDevice = (saph_ssd1306_device_t*) 0;
    uint8_t paramBuffer[] = {0xFA, 0xFB};
    uint32_t bufferSize = 2;
    int32_t errorCode = saph_ssd1306_internal_sendCtrlCommand(testDevice, paramBuffer, bufferSize);
    TEST_ASSERT_EQUAL_INT32(NULL_POINTER_ERROR, errorCode);
}

void test_saph_ssd1306_internal_sendCtrlCommand_returnsErrorOnBufferNullpointer(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    uint8_t* paramBuffer = (uint8_t*) 0;
    int32_t errorCode = saph_ssd1306_internal_sendCtrlCommand(&testDevice, paramBuffer, 1);
    TEST_ASSERT_EQUAL_INT32(NULL_POINTER_ERROR, errorCode);
}

void test_saph_ssd1306_internal_sendCtrlCommand_returnsErrorCodeForFailedWrite(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    uint8_t dc_byte = 0; // A command with only data
    uint8_t paramBuffer[] = {0xFA, 0xFB};
    uint32_t bufferSize = 2;
    uint8_t expectedData[3] = {dc_byte, paramBuffer[0], paramBuffer[1]};
    i2c_handler_write_ExpectAnyArgsAndReturn(WRITE_ERROR);
    int32_t errorCode = saph_ssd1306_internal_sendCtrlCommand(&testDevice, paramBuffer, bufferSize);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}
