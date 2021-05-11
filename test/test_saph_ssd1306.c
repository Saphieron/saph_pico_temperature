#include <stdbool.h>
#include "unity.h"

#include "saph_ssd1306.h"
#include "test_saph_ssd1306_test_definitions.h"
#include "mock_saph_ssd1306_internal.h"

// #############################################
// # Test group _init
// #############################################

#define ADDRESS_A 0x78
#define ADDRESS_B 0x79

void test_saph_ssd1306_init_savesAddressInDeviceStruct(void) {
    saph_ssd1306_device_t testDevice;
    uint8_t expectedAddr = ADDRESS_A;
    saph_ssd1306_init(expectedAddr, &testDevice);
    TEST_ASSERT_EQUAL_UINT8(expectedAddr, testDevice.address);
}

// #############################################
// # Test group _contrast
// #############################################

saph_ssd1306_device_t helper_createTestDevice(void) {
    saph_ssd1306_device_t testDevice;
    testDevice.address = ADDRESS_A;
    return testDevice;
}

void test_saph_ssd1306_contrast_setsValueUpTo256(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    uint8_t contrastLevel = 127;
    uint8_t commandAddress = 0x81;
    uint8_t expectedBuffer[] = {commandAddress, contrastLevel};
    saph_ssd1306_internal_sendCtrlCommand_ExpectAndReturn(&testDevice, expectedBuffer, 2, NO_ERROR);
    int32_t errorCode = saph_ssd1306_contrast(&testDevice, contrastLevel);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saph_ssd1306_contrast_returnsErrorOnDeviceNullPointer(void) {
    saph_ssd1306_device_t* nullPointerDevice = 0;
    int32_t errorCode = saph_ssd1306_contrast(nullPointerDevice, 0);
    TEST_ASSERT_EQUAL_INT32(NULL_POINTER_ERROR, errorCode);
}

void test_saph_ssd1306_contrast_returnsErrorOnFailedWrite(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    saph_ssd1306_internal_sendCtrlCommand_ExpectAnyArgsAndReturn(WRITE_ERROR);
    int32_t errorCode = saph_ssd1306_contrast(&testDevice, 0);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

// #############################################
// # Test group _displayOn
// #############################################

void test_saph_ssd1306_displayOn_canResumeAndIgnoreRamContents(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    uint8_t commandAddress = 0xA5;
    uint8_t expectedBuffer[] = {commandAddress};
    saph_ssd1306_internal_sendCtrlCommand_ExpectAndReturn(&testDevice, expectedBuffer, 1, NO_ERROR);
    bool ignoreRam = true;
    int32_t errorCode = saph_ssd1306_displayOn(&testDevice, ignoreRam);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saph_ssd1306_displayOn_canResumeAndUseRamContents(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    uint8_t commandAddress = 0xA4; //Last bit decides, whether to use the RAM contents or not
    uint8_t expectedBuffer[] = {commandAddress};
    saph_ssd1306_internal_sendCtrlCommand_ExpectAndReturn(&testDevice, expectedBuffer, 1, NO_ERROR);
    bool ignoreRam = false;
    int32_t errorCode = saph_ssd1306_displayOn(&testDevice, ignoreRam);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saph_ssd1306_displayOn_returnsErrorOnDeviceNullPointer(void) {
    saph_ssd1306_device_t* nullPointerDevice = 0;
    int32_t errorCode = saph_ssd1306_displayOn(nullPointerDevice, false);
    TEST_ASSERT_EQUAL_INT32(NULL_POINTER_ERROR, errorCode);
}

void test_saph_ssd1306_displayOn_returnsErrorOnFailedWrite(void) {
    saph_ssd1306_device_t testDevice = helper_createTestDevice();
    saph_ssd1306_internal_sendCtrlCommand_ExpectAnyArgsAndReturn(WRITE_ERROR);
    int32_t errorCode = saph_ssd1306_displayOn(&testDevice, false);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

// #############################################
// # Test group _displayOn
// #############################################

