#include "unity.h"

#include "saphBme280.h"

#include "mock_i2c_handler.h"

void setUp(void) {
}

void tearDown(void) {
}

/* *
 * TODO: make the register ctrl naming consistent
 * */

#define NO_ERROR 0

//#define COMM_ERROR_READ_AMOUNT -12

#define LOWEST_THREE_BITS 7

static saphBmeDevice_t helper_createBmeDevice(void) {
    uint8_t deviceAddr = 0xF7;
    saphBmeDevice_t bmeDevice = {deviceAddr};
    return bmeDevice;
}

// #############################################
// # Test group _init
// #############################################

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
    uint8_t expectedDeviceId = 0x60; //according to the BME280 datasheet
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    uint8_t response = expectedDeviceId;
    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ReturnArrayThruPtr_buffer(&response, 1);

    int32_t actualId = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT16(expectedDeviceId, actualId);
}

#define WRITE_ERROR -11

void test_saphBme280_getId_returnsErrorOnFailedWriteAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 0);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

#define READ_ERROR -12

void test_saphBme280_getId_returnsErrorOnFailedReadAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = READ_ERROR;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(0);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

#define ERROR_PLATFORM_GENERIC -2

void test_saphBme280_getId_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC; // Supposedly the generic error code for the pico
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, expectedError);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_getId_returnsI2cErrorCodeIfGenericHardwareErrorOnRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(expectedError);
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
    uint8_t buffer[2] = {resetRegisterAddr, registerValueForResetting};
    uint32_t amount = 2;
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, buffer, amount, amount, amount);
    int32_t result = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(SAPH_BME280_NO_ERROR, result);
}

void test_saphBme280_reset_returnsErrorOnFailedWriteAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    i2c_handler_write_ExpectAnyArgsAndReturn(0);
    int32_t errorCode = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_reset_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    i2c_handler_write_ExpectAnyArgsAndReturn(ERROR_PLATFORM_GENERIC);
    int32_t errorCode = saphBme280_resetDevice(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _prepareMeasureControlReg
// #############################################

#define TEST_OVERSAMPLING_SKIP 0x00
#define TEST_OVERSAMPLING_x1 0x01
#define TEST_OVERSAMPLING_x2 0x02
#define TEST_OVERSAMPLING_x4 0x03
#define TEST_OVERSAMPLING_x8 0x04
#define TEST_OVERSAMPLING_x16 0x05

void test_saphBme280_prepareMeasureControlReg_confirmOversamplingRates(void) {
    uint8_t allOversamplingValues[] = {OVERSAMPLING_SKIP, OVERSAMPLING_x1, OVERSAMPLING_x2, OVERSAMPLING_x4,
                                       OVERSAMPLING_x8, OVERSAMPLING_x16};
    uint8_t validationValues[] = {TEST_OVERSAMPLING_SKIP, TEST_OVERSAMPLING_x1, TEST_OVERSAMPLING_x2,
                                  TEST_OVERSAMPLING_x4, TEST_OVERSAMPLING_x8, TEST_OVERSAMPLING_x16};

    for (int i = 0; i < 6; ++i) {
        TEST_ASSERT_EQUAL_UINT8(validationValues[i], allOversamplingValues[i]);
    }
}

//Store a configuration to send over
void test_saphBme280_prepareMeasureControlReg_storesSettingsForLater(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t oversampling_x1 = TEST_OVERSAMPLING_x1; // from saphBme280.h
    uint8_t tempOversampling = oversampling_x1;
    uint8_t pressureOversampling = oversampling_x1;
    uint8_t deviceModeNormal = 3;
    uint8_t expectedUncommitedRegisterContent =
            (tempOversampling << 5) | (pressureOversampling << 2) | deviceModeNormal;

    saphBme280_prepareMeasureControlReg(&fakeDevice, tempOversampling, pressureOversampling, deviceModeNormal);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
}

#define SENSOR_MODE_SLEEP 0
#define SENSOR_MODE_FORCED 2
#define SENSOR_MODE_NORMAL 3

void test_saphBme280_prepareMeasureControlReg_checkAllOversamplingModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t amountSamplingOptions = 6;
    uint8_t allOversamplingValues[] = {OVERSAMPLING_SKIP, OVERSAMPLING_x1, OVERSAMPLING_x2, OVERSAMPLING_x4,
                                       OVERSAMPLING_x8, OVERSAMPLING_x16};
    for (uint8_t i = 0; i < amountSamplingOptions; ++i) {
        uint8_t expectedUncommitedRegisterContent =
                allOversamplingValues[i] << 5 | allOversamplingValues[i] << 2 | SENSOR_MODE_NORMAL;
        saphBme280_prepareMeasureControlReg(&fakeDevice, allOversamplingValues[i], allOversamplingValues[i],
                                            SENSOR_MODE_NORMAL);
        TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
    }

}

void test_saphBme280_prepareMeasureControlReg_checkAllSensorModes(void) {
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
        saphBme280_prepareMeasureControlReg(&fakeDevice, OVERSAMPLING_x16, OVERSAMPLING_x16,
                                            allSensorModes[i]);
        TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
    }
}

void test_saphBme280_prepareMeasureControlReg_onlyLowestThreeBitsUsedForOversampling(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t maskedOversamplingValue = tooLargeValue & 7;
    uint8_t expectedUncommitedRegisterContent =
            maskedOversamplingValue << 5 | OVERSAMPLING_x16 << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
    saphBme280_prepareMeasureControlReg(&fakeDevice, tooLargeValue, OVERSAMPLING_x16, SAPHBME280_SENSOR_MODE_NORMAL);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);

    expectedUncommitedRegisterContent =
            OVERSAMPLING_x16 << 5 | maskedOversamplingValue << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
    saphBme280_prepareMeasureControlReg(&fakeDevice, OVERSAMPLING_x16, tooLargeValue, SAPHBME280_SENSOR_MODE_NORMAL);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
}

void test_saphBme280_prepareMeasureControlReg_onlyLowestTwoBitsUsedForSensorModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t maskedOversamplingValue = tooLargeValue & 3;
    uint8_t expectedUncommitedRegisterContent =
            OVERSAMPLING_SKIP << 5 | OVERSAMPLING_SKIP << 2 | maskedOversamplingValue;
    saphBme280_prepareMeasureControlReg(&fakeDevice, OVERSAMPLING_SKIP, OVERSAMPLING_SKIP, tooLargeValue);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
}

// #############################################
// # Test group commitMeasureControlRegister
// #############################################

void test_saphBme280_commitMeasureControlRegister_sendsStoredRegisterValue(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBme280_prepareMeasureControlReg(&fakeDevice, OVERSAMPLING_x8, OVERSAMPLING_x8, SAPHBME280_SENSOR_MODE_NORMAL);
    uint8_t ctrlMeasureRegisterAddr = 0xF4;
    uint8_t expectedUncommitedRegisterContent =
            OVERSAMPLING_x8 << 5 | OVERSAMPLING_x8 << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
    uint8_t expectedBuffer[2] = {ctrlMeasureRegisterAddr, expectedUncommitedRegisterContent};
    uint8_t amountToSend = 2;
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, expectedBuffer, amountToSend, amountToSend,
                                               amountToSend);

    saphBme280_commitMeasureControlReg(&fakeDevice);
}

void test_saphBme280_commitMeasureControlRegister_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    int32_t differingReturnValue = 1;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);

    int32_t errorCode = saphBme280_commitMeasureControlReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_commitMeasureControlRegister_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    int32_t wrongReturnValue = ERROR_PLATFORM_GENERIC;
    i2c_handler_write_ExpectAnyArgsAndReturn(wrongReturnValue);

    int32_t errorCode = saphBme280_commitMeasureControlReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _prepareConfigReg
// #############################################

#define STANDBY_TIME_MS_0_5 0x00
#define STANDBY_TIME_MS_62_5 0x01
#define STANDBY_TIME_MS_125_0 0x02
#define STANDBY_TIME_MS_250_0 0x03
#define STANDBY_TIME_MS_500_0 0x04
#define STANDBY_TIME_MS_1000_0 0x05
#define STANDBY_TIME_MS_10_0 0x06
#define STANDBY_TIME_MS_20_0 0x07

#define IIR_FILTER_COEFFICIENT_OFF 0x00
#define IIR_FILTER_COEFFICIENT_2 0x01
#define IIR_FILTER_COEFFICIENT_4 0x02
#define IIR_FILTER_COEFFICIENT_8 0x03
#define IIR_FILTER_COEFFICIENT_16 0x04

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
    saphBme280_prepareConfigurationReg(&fakeDevice, standbyTime, iirFilterCoefficient);

    uint8_t expectedRegValue = standbyTime << 5 | iirFilterCoefficient << 2;
    TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
}

void test_saphBme280_prepareConfig_onlyLowestThreeBitsUsedForStandbyTime(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t iirFilterCoefficient = SAPHBME280_IIR_FILTER_COEFFICIENT_8;
    saphBme280_prepareConfigurationReg(&fakeDevice, tooLargeValue, iirFilterCoefficient);

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
        saphBme280_prepareConfigurationReg(&fakeDevice, allStandbyTimes[i], SAPHBME280_IIR_FILTER_COEFFICIENT_16);
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
        saphBme280_prepareConfigurationReg(&fakeDevice, SAPHBME280_STANDBY_TIME_MS_125_0, allIirCoefficients[i]);
        uint8_t expectedRegValue = SAPHBME280_STANDBY_TIME_MS_125_0 << 5 | allIirCoefficients[i] << 2;
        TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
    }
}

// #############################################
// # Test group _commitConfigRegister
// #############################################

void test_saphBme280_commitConfigRegister_sendsStoredRegisterValue(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBme280_prepareConfigurationReg(&fakeDevice, SAPHBME280_STANDBY_TIME_MS_500_0,
                                       SAPHBME280_IIR_FILTER_COEFFICIENT_2);

    uint8_t configRegisterAddr = 0xF5;
    uint8_t expectedUncommitedRegisterContent =
            SAPHBME280_STANDBY_TIME_MS_500_0 << 5 | SAPHBME280_IIR_FILTER_COEFFICIENT_2 << 2;
    uint8_t expectedBuffer[2] = {configRegisterAddr, expectedUncommitedRegisterContent};
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, expectedBuffer, 2, 2,
                                               2);

    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(SAPH_BME280_NO_ERROR, errorCode);
}

void test_saphBme280_commitConfigRegister_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = WRITE_ERROR;
    uint8_t differingReturnValue = 1;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);
    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_commitConfigRegister_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    int32_t differingReturnValue = ERROR_PLATFORM_GENERIC;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);
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
    uint8_t expectedBuffer[2] = {configRegisterAddr, expectedUncommitedRegisterContent};
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, expectedBuffer, 2, 2,
                                               2);

    int32_t errorCode = saphBme280_commitCtrlHumidity(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saphBme280_commitCtrlHumidity_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t differingReturnValue = 1;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);
    int32_t errorCode = saphBme280_commitCtrlHumidity(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

void test_saphBme280_commitCtrlHumidity_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    int32_t differingReturnValue = ERROR_PLATFORM_GENERIC;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);
    int32_t errorCode = saphBme280_commitCtrlHumidity(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

// #############################################
// # Test group _getstatus
// #############################################

void test_saphBme280_getstatus_returnsStatusByteThroughPointer(void) {
    uint8_t statusRegisterAddr = 0xF3;
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &statusRegisterAddr, 1, 1, 1);

    uint8_t response = (1 << 3) | (1 << 0);

    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ReturnArrayThruPtr_buffer(&response, 1);
    uint8_t status = 0xFF;
    uint8_t errorCode = saphBme280_status(&fakeDevice, &status);

    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_UINT8(response, status);
}

void test_saphBme280_getstatus_returnsErrorOnWrongAmountWritten(void) {
    uint8_t statusRegisterAddr = 0xF3;
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &statusRegisterAddr, 1, 1, 0);
    uint8_t untouchedBuffer = 0xFF;
    int32_t errorCode = saphBme280_status(&fakeDevice, &untouchedBuffer);

    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
    TEST_ASSERT_EQUAL_UINT8(0xFF, untouchedBuffer);
}

void test_saphBme280_getstatus_returnsErrorOnWrongAmountRead(void) {
    uint8_t statusRegisterAddr = 0xF3;
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &statusRegisterAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(0);
    uint8_t untouchedBuffer = 0xFF;
    int32_t errorCode = saphBme280_status(&fakeDevice, &untouchedBuffer);

    TEST_ASSERT_EQUAL_INT32(READ_ERROR, errorCode);
    TEST_ASSERT_EQUAL_UINT8(0xFF, untouchedBuffer);
}

// #############################################
// # Test group _getAllMeasurements
// #############################################

#define MEASUREMENT_SIZE 8

void test_saphBme280_getRawAllMeasurements_returnsPressureThroughStruct(void) {
    // TODO: In general the raw value appears to be wrong as other code works with something in the range of 528112
    // TODO: Kind of twice the amount i was seeing here. Now what

//    int8_t something = 0x0F;
//    printf("0x%2X\n", something << 3);
//    TEST_FAIL_MESSAGE("the xlsb bits are not taken properly from 0xF9");
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t startingRegister = 0xF7; // the pressure register
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &startingRegister, 1, 1, 1);
    uint8_t response[] = {0xFF, 0x00, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int32_t expectedPressure = (response[0] << 12) + (response[1] << 4) + (response[2] >> 4);
    saphBmeRawMeasurements_t result;
    result.pressure = 0;

    i2c_handler_read_ExpectAnyArgsAndReturn(MEASUREMENT_SIZE);
    i2c_handler_read_ReturnArrayThruPtr_buffer(response, MEASUREMENT_SIZE);
    int32_t errorCode = saphBme280_getRawMeasurement(&fakeDevice, &result);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_INT32(expectedPressure, result.pressure);
}

void test_saphBme280_getRawAllMeasurements_returnsTemperatureThroughStruct(void) {
//    TEST_FAIL_MESSAGE("the xlsb bits are not taken properly from 0xFC");
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t startingRegister = 0xF7; // the pressure register
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &startingRegister, 1, 1, 1);
    uint8_t response[] = {0x00, 0x00, 0x00, 0xFF, 0x00, 0xD0, 0x00, 0x00};
    int32_t expectedTemp = (response[3] << 12) + (response[4] << 4) + (response[5] >> 4);
    saphBmeRawMeasurements_t result;
    result.temperature = 0;

    i2c_handler_read_ExpectAnyArgsAndReturn(MEASUREMENT_SIZE);
    i2c_handler_read_ReturnArrayThruPtr_buffer(response, MEASUREMENT_SIZE);
    int32_t errorCode = saphBme280_getRawMeasurement(&fakeDevice, &result);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_INT32(expectedTemp, result.temperature);
}

void test_saphBme280_getRawAllMeasurements_returnsHumidityInItsPointers(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t startingRegister = 0xF7; // the pressure register
    uint8_t response[] = {0xAC, 0xAB, 0xAA, 0xBC, 0xBB, 0xBA, 0xCB, 0xCA};
    int32_t expectedHumidity = (response[6] << 8) + (response[7]);
    saphBmeRawMeasurements_t result;
    result.humidity = 0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &startingRegister, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(MEASUREMENT_SIZE);
    i2c_handler_read_ReturnArrayThruPtr_buffer(response, MEASUREMENT_SIZE);
    int32_t errorCode = saphBme280_getRawMeasurement(&fakeDevice, &result);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_INT32(expectedHumidity, result.humidity);
}

void test_saphBme280_getRawAllMeasurements_returnsErrorCodeForFailedWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    void* nothingness = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(0);
    int32_t result = saphBme280_getRawMeasurement(&fakeDevice, nothingness);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, result);
}

void test_saphBme280_getRawAllMeasurements_returnsErrorCodeForFailedRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    void* nothingness = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    int32_t result = saphBme280_getRawMeasurement(&fakeDevice, nothingness);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, result);
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

// #############################################
// # Test group _readTrimmingValues
// #############################################

#define firstBurstReadAmount 25
#define secondBurstReadAmount 1
#define thirdBurstReadAmount 7
uint8_t trimmingFirstResponse[firstBurstReadAmount];
uint8_t trimmingSecondResponse[secondBurstReadAmount];
uint8_t trimmingThirdResponse[thirdBurstReadAmount];

#define BURST_ADDR_FIRST 0x88

static void helper_prepareI2cBurstRead(saphBmeDevice_t* fakeDevice) {
    uint8_t firstStartAddr = BURST_ADDR_FIRST;
    uint8_t secondStartAddr = 0xA1;
    uint8_t thirdStartAddr = 0xE1;
    for (int i = 0; i < firstBurstReadAmount; ++i) {
        trimmingFirstResponse[i] = i;
    }

    for (int i = 0; i < secondBurstReadAmount; ++i) {
        trimmingSecondResponse[i] = 0xBB + i;
    }

    for (int i = 0; i < thirdBurstReadAmount; ++i) {
        trimmingThirdResponse[i] = 0xCC + i;
    }
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice->address, &firstStartAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(firstBurstReadAmount);
    i2c_handler_read_ReturnArrayThruPtr_buffer(trimmingFirstResponse, firstBurstReadAmount);

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice->address, &secondStartAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(secondBurstReadAmount);
    i2c_handler_read_ReturnArrayThruPtr_buffer(trimmingSecondResponse, secondBurstReadAmount);

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice->address, &thirdStartAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(thirdBurstReadAmount);
    i2c_handler_read_ReturnArrayThruPtr_buffer(trimmingThirdResponse, thirdBurstReadAmount);
}

static void helper_checkUnsignedTrimmingValue(const uint8_t* expectedValue, const uint16_t* actual) {
    uint16_t expectedResult = (expectedValue[1] << 8) + expectedValue[0];
    TEST_ASSERT_EQUAL_UINT16(expectedResult, *actual);
}

static void helper_checkSignedTrimmingValue(const uint8_t* expectedValue, const int16_t* actual) {
    uint16_t expectedDigT1 = (expectedValue[1] << 8) + expectedValue[0];
    TEST_ASSERT_EQUAL_UINT16(expectedDigT1, *actual);
}

void test_saphBme280_readTrimmingValues_burstReadInThreeSteps(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_checkTemperatureTrimmingValues(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    saphBme280_readTrimmingValues(&fakeDevice);
    saphBmeTrimmingValues_t trimmingValues = fakeDevice.trimmingValues;
    helper_checkUnsignedTrimmingValue(trimmingFirstResponse, &(trimmingValues.dig_T1));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 2, &(trimmingValues.dig_T2));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 4, &(trimmingValues.dig_T3));
}

void test_saphBme280_readTrimmingValues_checkPressureTrimmingValues(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    saphBme280_readTrimmingValues(&fakeDevice);
    saphBmeTrimmingValues_t trimmingValues = fakeDevice.trimmingValues;

    // Yes i'm aware this isn't sexy, but it is expressive about what and where each value is.
    helper_checkUnsignedTrimmingValue(trimmingFirstResponse + 6, &(trimmingValues.dig_P1));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 8, &(trimmingValues.dig_P2));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 10, &(trimmingValues.dig_P3));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 12, &(trimmingValues.dig_P4));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 14, &(trimmingValues.dig_P5));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 16, &(trimmingValues.dig_P6));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 18, &(trimmingValues.dig_P7));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 20, &(trimmingValues.dig_P8));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 22, &(trimmingValues.dig_P9));
}

#define LOWER_FOUR_BITS 0x0F

void test_saphBme280_readTrimmingValues_checkHumidityTrimmingValues(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    saphBme280_readTrimmingValues(&fakeDevice);

    saphBmeTrimmingValues_t trimmingValues = fakeDevice.trimmingValues;
    TEST_ASSERT_EQUAL_UINT8(trimmingSecondResponse[0], trimmingValues.dig_H1);

    int16_t expectedH2 = (trimmingThirdResponse[1] << 8) + trimmingThirdResponse[0];
    TEST_ASSERT_EQUAL_INT16(expectedH2, trimmingValues.dig_H2);

    TEST_ASSERT_EQUAL_UINT8(trimmingThirdResponse[2], trimmingValues.dig_H3);

    int16_t expectedH4 = (trimmingThirdResponse[3] << 4) + (trimmingThirdResponse[4] & LOWER_FOUR_BITS);
    TEST_ASSERT_EQUAL_INT16(expectedH4, trimmingValues.dig_H4);

    int16_t expectedH5 = (trimmingThirdResponse[5] << 4) + (trimmingThirdResponse[4] >> 4);
    TEST_ASSERT_EQUAL_INT16(expectedH5, trimmingValues.dig_H5);

    TEST_ASSERT_EQUAL_INT8(trimmingThirdResponse[6], trimmingValues.dig_H6);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedFirstWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t wrongWriteAmount = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(wrongWriteAmount);

    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedFirstRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    int32_t wrongReadAmount = 0;
    i2c_handler_read_ExpectAnyArgsAndReturn(wrongReadAmount);

    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedSecondWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(25);
    int32_t wrongWriteAmount = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(wrongWriteAmount);
    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedSecondRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(25);
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    int32_t wrongReadAmount = 0;
    i2c_handler_read_ExpectAnyArgsAndReturn(wrongReadAmount);
    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedThirdWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(25);
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    int32_t wrongWriteAmount = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(wrongWriteAmount);
    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedThirdRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(25);
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    int32_t wrongReadAmount = 0;
    i2c_handler_read_ExpectAnyArgsAndReturn(wrongReadAmount);
    int32_t errorCode = saphBme280_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, errorCode);
}

