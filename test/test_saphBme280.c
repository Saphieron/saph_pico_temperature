#include "saphBme280.h"
#include "unity.h"

#include "test_saphBme280_test_definitions.h"
#include "mock_i2c_handler.h"
#include "mock_saphBme280_internal.h"

/* *
 * If a symbol origin is unclear it is defined in test_saphBme280_test_definitions.h
 * TODO: make the register ctrl naming consistent
 * */

static saphBmeDevice_t helper_createBmeDevice(void) {
    uint8_t deviceAddr = 0xF0;
    saphBmeDevice_t bmeDevice = {deviceAddr};
    return bmeDevice;
}

// #############################################
// # Test group _init
// #############################################

void test_saphBme280_init_initialisesAPassedDevicePointer(void) {
    // Might add some more config settings to it
    uint8_t deviceAddr = 0xFF;
    saphBmeDevice_t actualDevice;
    saphBme280_internal_readTrimmingValues_ExpectAndReturn(&actualDevice, NO_ERROR);

    int32_t errorCode = saphBme280_init(deviceAddr, &actualDevice);
    TEST_ASSERT_EQUAL_UINT8(deviceAddr, actualDevice.address);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saphBme280_init_returnsErrorIfDeviceIsNull(void) {
    uint8_t deviceAddr = 0xFF;
    int32_t expectedErrorCode = NULL_POINTER_ERROR;
    saphBmeDevice_t* nullDevice = (saphBmeDevice_t*) 0;
    int32_t errorCode = saphBme280_init(deviceAddr, nullDevice);
    TEST_ASSERT_EQUAL_INT32(expectedErrorCode, errorCode);
}

// Reserved according to https://www.i2c-bus.org/
void test_saphBme280_init_returnsErrorIfPassedReservedI2CAddress(void) {
    uint8_t reservedAddresses[] = {0x00, 0x01, 0x02, 0x03};
    int32_t expectedErrorCode = ADDRESS_RESERVED_ERROR;
    saphBmeDevice_t testDevice;
    for (int i = 0; i < 4; ++i) {
        int32_t errorCode = saphBme280_init(reservedAddresses[i], &testDevice);
        TEST_ASSERT_EQUAL_INT32(expectedErrorCode, errorCode);
    }
}

void test_saphBme280_init_returnsErrorOnFailedWriteAmount(void) {
    saphBmeDevice_t testDevice;
    uint8_t deviceAddr = 0xFF;
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_readTrimmingValues_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_init(deviceAddr, &testDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_init_returnsErrorOnFailedReadAmount(void) {
    saphBmeDevice_t fakeDevice;
    uint8_t deviceAddr = 0xFF;
    int32_t expectedError = READ_ERROR;
    saphBme280_internal_readTrimmingValues_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_init(deviceAddr, &fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_init_returnsErrorOnGenericPlatformError(void) {
    saphBmeDevice_t fakeDevice;
    uint8_t deviceAddr = 0xFF;
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    saphBme280_internal_readTrimmingValues_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_init(deviceAddr, &fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _getId
// #############################################

void test_saphBme280_getId(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t expectedDeviceId = 0x60; //according to the BME280 datasheet
    uint8_t idRegisterAddr = 0xD0;
    uint8_t someBuffer = 0;
    saphBme280_internal_readFromRegister_ExpectAndReturn(&fakeDevice, idRegisterAddr, &someBuffer, 1, NO_ERROR);
    uint8_t theResponse = expectedDeviceId;
    saphBme280_internal_readFromRegister_ReturnThruPtr_readingBuffer(&theResponse);
    int32_t actualId = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedDeviceId, actualId);
}

void test_saphBme280_getId_returnsErrorOnFailedWriteAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_readFromRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_getId_returnsErrorOnFailedReadAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = READ_ERROR;
    saphBme280_internal_readFromRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_getId_returnsI2cErrorCodeIfGenericHardwareError(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC; // Supposedly the generic error code for the pico
    saphBme280_internal_readFromRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _resetDevice
// #############################################

void test_saphBme280_reset_deviceIsReset(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t resetRegisterAddr = 0xE0;
    uint8_t registerValueForResetting = 0xB6;
    uint32_t bufferSize = 2;
    uint8_t buffer[] = {resetRegisterAddr, registerValueForResetting};
    saphBme280_internal_writeToRegister_ExpectAndReturn(&fakeDevice, buffer, bufferSize, NO_ERROR);
    int32_t result = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(SAPH_BME280_NO_ERROR, result);
}

void test_saphBme280_reset_returnsErrorOnFailedWriteAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_reset_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _prepareMeasureCtrlReg
// #############################################

//This is simply a small check to just define the appropriate magic values
void test_saphBme280_prepareMeasureCtrlReg_confirmOversamplingRates(void) {
    uint8_t allOversamplingValues[] = {OVERSAMPLING_SKIP, OVERSAMPLING_x1, OVERSAMPLING_x2, OVERSAMPLING_x4,
                                       OVERSAMPLING_x8, OVERSAMPLING_x16};
    uint8_t validationValues[] = {TEST_OVERSAMPLING_SKIP, TEST_OVERSAMPLING_x1, TEST_OVERSAMPLING_x2,
                                  TEST_OVERSAMPLING_x4, TEST_OVERSAMPLING_x8, TEST_OVERSAMPLING_x16};

    for (int i = 0; i < 6; ++i) {
        TEST_ASSERT_EQUAL_UINT8(validationValues[i], allOversamplingValues[i]);
    }
}

//Store a configuration before sending it over
void test_saphBme280_prepareMeasureCtrlReg_storesSettingsForLater(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t oversampling_x1 = TEST_OVERSAMPLING_x1; // from saphBme280.h
    uint8_t tempOversampling = oversampling_x1;
    uint8_t pressureOversampling = oversampling_x1;
    uint8_t deviceModeNormal = 3;
    uint8_t expectedUncommitedRegisterContent =
            (tempOversampling << 5) | (pressureOversampling << 2) | deviceModeNormal;

    saphBme280_prepareMeasureCtrlReg(&fakeDevice, tempOversampling, pressureOversampling, deviceModeNormal);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureCtrl);
}

void test_saphBme280_prepareMeasureCtrlReg_checkAllOversamplingModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t amountSamplingOptions = 6;
    uint8_t allOversamplingValues[] = {OVERSAMPLING_SKIP, OVERSAMPLING_x1, OVERSAMPLING_x2, OVERSAMPLING_x4,
                                       OVERSAMPLING_x8, OVERSAMPLING_x16};
    for (uint8_t i = 0; i < amountSamplingOptions; ++i) {
        uint8_t expectedUncommitedRegisterContent =
                allOversamplingValues[i] << 5 | allOversamplingValues[i] << 2 | SENSOR_MODE_NORMAL;
        saphBme280_prepareMeasureCtrlReg(&fakeDevice, allOversamplingValues[i], allOversamplingValues[i],
                                         SENSOR_MODE_NORMAL);
        TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureCtrl);
    }
}

void test_saphBme280_prepareMeasureCtrlReg_checkAllSensorModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t allSensorModes[] = {SAPHBME280_SENSOR_MODE_SLEEP, SAPHBME280_SENSOR_MODE_FORCED,
                                SAPHBME280_SENSOR_MODE_NORMAL};
    uint8_t validationValues[] = {SENSOR_MODE_SLEEP, SENSOR_MODE_FORCED,
                                  SENSOR_MODE_NORMAL};
    uint8_t amountSensorModes = 3;
    for (int i = 0; i < amountSensorModes; ++i) {
        TEST_ASSERT_EQUAL_UINT8(validationValues[i], allSensorModes[i]);
        uint8_t expectedUncommitedRegisterContent =
                OVERSAMPLING_x16 << 5 | OVERSAMPLING_x16 << 2 | allSensorModes[i];
        saphBme280_prepareMeasureCtrlReg(&fakeDevice, OVERSAMPLING_x16, OVERSAMPLING_x16,
                                         allSensorModes[i]);
        TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureCtrl);
    }
}

void test_saphBme280_prepareMeasureCtrlReg_onlyLowestThreeBitsUsedForOversampling(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t maskedOversamplingValue = tooLargeValue & 7;
    uint8_t expectedUncommitedRegisterContent =
            maskedOversamplingValue << 5 | OVERSAMPLING_x16 << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
    saphBme280_prepareMeasureCtrlReg(&fakeDevice, tooLargeValue, OVERSAMPLING_x16, SAPHBME280_SENSOR_MODE_NORMAL);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureCtrl);

    expectedUncommitedRegisterContent =
            OVERSAMPLING_x16 << 5 | maskedOversamplingValue << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
    saphBme280_prepareMeasureCtrlReg(&fakeDevice, OVERSAMPLING_x16, tooLargeValue, SAPHBME280_SENSOR_MODE_NORMAL);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureCtrl);
}

void test_saphBme280_prepareMeasureCtrlReg_onlyLowestTwoBitsUsedForSensorModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t maskedOversamplingValue = tooLargeValue & 3;
    uint8_t expectedUncommitedRegisterContent =
            OVERSAMPLING_SKIP << 5 | OVERSAMPLING_SKIP << 2 | maskedOversamplingValue;
    saphBme280_prepareMeasureCtrlReg(&fakeDevice, OVERSAMPLING_SKIP, OVERSAMPLING_SKIP, tooLargeValue);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureCtrl);
}

// #############################################
// # Test group commitMeasureCtrlReg
// #############################################

void test_saphBme280_commitMeasureCtrlReg_sendsStoredRegisterValue(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBme280_prepareMeasureCtrlReg(&fakeDevice, OVERSAMPLING_x8, OVERSAMPLING_x8, SAPHBME280_SENSOR_MODE_NORMAL);
    uint8_t ctrlMeasureRegisterAddr = 0xF4;
    uint8_t expectedUncommitedRegisterContent =
            OVERSAMPLING_x8 << 5 | OVERSAMPLING_x8 << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
    uint8_t amountToSend = 2;
    uint8_t expectedBuffer[] = {ctrlMeasureRegisterAddr, expectedUncommitedRegisterContent};
    saphBme280_internal_writeToRegister_ExpectAndReturn(&fakeDevice, expectedBuffer, amountToSend,
                                                        SAPH_BME280_NO_ERROR);
    saphBme280_commitMeasureCtrlReg(&fakeDevice);
}

void test_saphBme280_commitMeasureCtrlReg_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);

    int32_t errorCode = saphBme280_commitMeasureCtrlReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_commitMeasureCtrlReg_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);

    int32_t errorCode = saphBme280_commitMeasureCtrlReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _prepareConfigReg
// #############################################

void test_saphBme280_prepareConfigReg_confirmStandbyAndIirFilterValues(void) {
    uint8_t allStandbyTimes[] = {SAPHBME280_STANDBY_TIME_MS_0_5, SAPHBME280_STANDBY_TIME_MS_62_5,
                                 SAPHBME280_STANDBY_TIME_MS_125_0, SAPHBME280_STANDBY_TIME_MS_250_0,
                                 SAPHBME280_STANDBY_TIME_MS_500_0, SAPHBME280_STANDBY_TIME_MS_1000_0,
                                 SAPHBME280_STANDBY_TIME_MS_10_0, SAPHBME280_STANDBY_TIME_MS_20_0};
    uint8_t validationValuesStandbyTimes[] = {STANDBY_TIME_MS_0_5, STANDBY_TIME_MS_62_5, STANDBY_TIME_MS_125_0,
                                              STANDBY_TIME_MS_250_0, STANDBY_TIME_MS_500_0, STANDBY_TIME_MS_1000_0,
                                              STANDBY_TIME_MS_10_0, STANDBY_TIME_MS_20_0};
    for (int i = 0; i < 8; ++i) {
        TEST_ASSERT_EQUAL_UINT8(validationValuesStandbyTimes[i], allStandbyTimes[i]);
    }
    uint8_t allIirCoefficients[] = {SAPHBME280_IIR_FILTER_COEFFICIENT_OFF, SAPHBME280_IIR_FILTER_COEFFICIENT_2,
                                    SAPHBME280_IIR_FILTER_COEFFICIENT_4, SAPHBME280_IIR_FILTER_COEFFICIENT_8,
                                    SAPHBME280_IIR_FILTER_COEFFICIENT_16};
    uint8_t validationValuesIirCoefficients[] = {IIR_FILTER_COEFFICIENT_OFF, IIR_FILTER_COEFFICIENT_2,
                                                 IIR_FILTER_COEFFICIENT_4, IIR_FILTER_COEFFICIENT_8,
                                                 IIR_FILTER_COEFFICIENT_16};
    for (int j = 0; j < 5; ++j) {
        TEST_ASSERT_EQUAL_UINT8(validationValuesIirCoefficients[j], allIirCoefficients[j]);
    }
}

void test_saphBme280_prepareConfigReg_definesStandbyPeriodAndIirFilter(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t standbyTime = STANDBY_TIME_MS_1000_0;
    uint8_t iirFilterCoefficient = SAPHBME280_IIR_FILTER_COEFFICIENT_8;
    saphBme280_prepareConfigReg(&fakeDevice, standbyTime, iirFilterCoefficient);

    uint8_t expectedRegValue = standbyTime << 5 | iirFilterCoefficient << 2;
    TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
}

void test_saphBme280_prepareConfig_onlyLowestThreeBitsUsedForStandbyTime(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t iirFilterCoefficient = SAPHBME280_IIR_FILTER_COEFFICIENT_8;
    saphBme280_prepareConfigReg(&fakeDevice, tooLargeValue, iirFilterCoefficient);

    uint8_t maskedStandbyTimeValue = tooLargeValue & 7;
    uint8_t expectedRegValue = maskedStandbyTimeValue << 5 | iirFilterCoefficient << 2;
    TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
}

void test_saphBme280_prepareConfig_checkAllStandbyTimes(void) {
    uint8_t amountStandbyTimes = 8;
    uint8_t allStandbyTimes[] = {SAPHBME280_STANDBY_TIME_MS_0_5, SAPHBME280_STANDBY_TIME_MS_62_5,
                                 SAPHBME280_STANDBY_TIME_MS_125_0, SAPHBME280_STANDBY_TIME_MS_250_0,
                                 SAPHBME280_STANDBY_TIME_MS_500_0, SAPHBME280_STANDBY_TIME_MS_1000_0,
                                 SAPHBME280_STANDBY_TIME_MS_10_0, SAPHBME280_STANDBY_TIME_MS_20_0};
    for (uint8_t i = 0; i < amountStandbyTimes; ++i) {
        saphBmeDevice_t fakeDevice = helper_createBmeDevice();
        saphBme280_prepareConfigReg(&fakeDevice, allStandbyTimes[i], SAPHBME280_IIR_FILTER_COEFFICIENT_16);
        uint8_t expectedRegValue = allStandbyTimes[i] << 5 | SAPHBME280_IIR_FILTER_COEFFICIENT_16 << 2;
        TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
    }
}

void test_saphBme280_prepareConfig_checkAllIirCoefficients(void) {
    uint8_t amountIirCoefficients = 5;
    uint8_t allIirCoefficients[] = {SAPHBME280_IIR_FILTER_COEFFICIENT_OFF, SAPHBME280_IIR_FILTER_COEFFICIENT_2,
                                    SAPHBME280_IIR_FILTER_COEFFICIENT_4, SAPHBME280_IIR_FILTER_COEFFICIENT_8,
                                    SAPHBME280_IIR_FILTER_COEFFICIENT_16};
    for (uint8_t i = 0; i < amountIirCoefficients; ++i) {
        saphBmeDevice_t fakeDevice = helper_createBmeDevice();
        saphBme280_prepareConfigReg(&fakeDevice, SAPHBME280_STANDBY_TIME_MS_125_0, allIirCoefficients[i]);
        uint8_t expectedRegValue = SAPHBME280_STANDBY_TIME_MS_125_0 << 5 | allIirCoefficients[i] << 2;
        TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
    }
}

// #############################################
// # Test group _commitConfigReg
// #############################################

void test_saphBme280_commitConfigRegister_sendsStoredRegisterValue(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBme280_prepareConfigReg(&fakeDevice, SAPHBME280_STANDBY_TIME_MS_500_0, SAPHBME280_IIR_FILTER_COEFFICIENT_2);
    uint8_t configRegisterAddr = 0xF5;
    uint8_t expectedUncommitedRegisterContent =
            SAPHBME280_STANDBY_TIME_MS_500_0 << 5 | SAPHBME280_IIR_FILTER_COEFFICIENT_2 << 2;
    uint32_t amount = 2;
    uint8_t expectedBuffer[2] = {configRegisterAddr, expectedUncommitedRegisterContent};
    saphBme280_internal_writeToRegister_ExpectAndReturn(&fakeDevice, expectedBuffer, amount, NO_ERROR);

    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(SAPH_BME280_NO_ERROR, errorCode);
}

void test_saphBme280_commitConfigRegister_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_commitConfigRegister_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _prepareCtrlHumidity
// #############################################

void test_saphBme280_prepareCtrlHumidity_storesSettingsForLater(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t humidityOversampling = TEST_OVERSAMPLING_x1;
    uint8_t expectedUncommitedRegisterContent = humidityOversampling;

    saphBme280_prepareCtrlHumidityReg(&fakeDevice, humidityOversampling);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerCtrlHumidity);
}

void test_saphBme280_prepareCtrlHumidity_masksUnrelatedBitsInArgument(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t humidityOversampling = 0xF8 | TEST_OVERSAMPLING_x4;
    uint8_t expectedUncommitedRegisterContent = humidityOversampling & LOWEST_THREE_BITS;

    saphBme280_prepareCtrlHumidityReg(&fakeDevice, humidityOversampling);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerCtrlHumidity);
}

// #############################################
// # Test group _commitCtrlHumidity
// #############################################

void test_saphBme280_commitCtrlHumidity_sendsStoredRegisterValue(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBme280_prepareCtrlHumidityReg(&fakeDevice, TEST_OVERSAMPLING_x8);
    uint8_t configRegisterAddr = 0xF2;
    uint8_t expectedUncommitedRegisterContent = fakeDevice.registerCtrlHumidity;
    uint32_t bufferSize = 2;
    uint8_t expectedBuffer[2] = {configRegisterAddr, expectedUncommitedRegisterContent};
    saphBme280_internal_writeToRegister_ExpectAndReturn(&fakeDevice, expectedBuffer, bufferSize, NO_ERROR);

    int32_t errorCode = saphBme280_commitCtrlHumidity(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saphBme280_commitCtrlHumidity_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_commitCtrlHumidity(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_commitCtrlHumidity_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    saphBme280_internal_writeToRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_commitCtrlHumidity(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _getstatus
// #############################################

void test_saphBme280_getstatus_returnsStatusByteThroughPointer(void) {
    uint8_t statusRegisterAddr = 0xF3;
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t passedBuffer = 0xff;

    saphBme280_internal_readFromRegister_ExpectAndReturn(&fakeDevice, statusRegisterAddr, &passedBuffer, 1, NO_ERROR);
    uint8_t response = (1 << 3) | (1 << 0);;
    saphBme280_internal_readFromRegister_ReturnThruPtr_readingBuffer(&response);
    uint8_t errorCode = saphBme280_status(&fakeDevice, &passedBuffer);

    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_UINT8(response, passedBuffer);
}

void test_saphBme280_getstatus_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    saphBme280_internal_readFromRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_status(&fakeDevice, 0);

    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_getstatus_returnsErrorOnWrongAmountRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = READ_ERROR;
    saphBme280_internal_readFromRegister_ExpectAnyArgsAndReturn(expectedError);
    int32_t errorCode = saphBme280_status(&fakeDevice, 0);

    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _getMeasurements
// #############################################
void test_saphBme280_getMeasurements_returnsAllValuesAfterTrimming(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBmeRawMeasurements_t rawMeasurementBuffer = {0, 0, 0};
    saphBme280_internal_getRawMeasurement_ExpectAndReturn(&fakeDevice, &rawMeasurementBuffer, NO_ERROR);
    saphBmeMeasurements_t compensatedMeasurements = {2189, 25821489, 39671};
    saphBme280_internal_compensateMeasurements_ExpectAndReturn(&fakeDevice, &rawMeasurementBuffer,
                                                               compensatedMeasurements);
    saphBmeMeasurements_t result = {0, 0, 0};
    int32_t errorCode = saphBme280_getMeasurements(&fakeDevice, &result);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);

    TEST_ASSERT_EQUAL_UINT32(compensatedMeasurements.pressure, result.pressure);
    TEST_ASSERT_EQUAL_INT32(compensatedMeasurements.temperature, result.temperature);
    TEST_ASSERT_EQUAL_UINT32(compensatedMeasurements.humidity, result.humidity);
}
// #############################################
// # Test group _getPressure
// #############################################

//void test_saphBme280_getPressure(void) {
//    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
//    uint8_t pressureRegAddress = 0xF7;
//    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &pressureRegAddress, 1, 1, 1);
//    uint8_t receivedResponse[] = {0xFA, 0xFB, 0xFC};
//    uint32_t pressureValue = 0;
//    i2c_handler_read_ExpectAndReturn(fakeDevice.address, (uint8_t*) &pressureValue, 3, 3);
//    i2c_handler_read_ReturnArrayThruPtr_buffer(receivedResponse, 3);
//
//    int32_t errorCode = saphBme280_getPressure(&fakeDevice, &pressureValue);
//    uint32_t expectedPressure = receivedResponse[0] << 12 | receivedResponse[1] << 4 | (receivedResponse[2] & 0x07);
//    TEST_ASSERT_EQUAL_UINT32(expectedPressure, pressureValue);
//    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
//}
//
