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
//#define COMM_ERROR_READ_AMOUNT -12

saphBmeDevice_t helper_createBmeDevice(void) {
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

void test_saphBme280_getId_returnsErrorOnFailedWriteAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = SAPH_BME280_COMM_ERROR_WRITE_AMOUNT;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 0);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_getId_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC; // Supposedly the generic error code for the pico
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, expectedError);
    int32_t errorCode = saphBme280_getId(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_getId_returnsErrorOnFailedReadAmount(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = SAPH_BME280_COMM_ERROR_READ_AMOUNT;
    uint8_t idRegisterAddr = 0xD0;

    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &idRegisterAddr, 1, 1, 1);
    i2c_handler_read_ExpectAnyArgsAndReturn(0);
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
    int32_t expectedError = SAPH_BME280_COMM_ERROR_WRITE_AMOUNT;
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

//Store a configuration to send over
void test_saphBme280_prepareMeasureControlReg_storesSettingsForLater(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t oversampling_x1 = OVERSAMPLING_x1; // from saphBme280.h
    uint8_t tempOversampling = oversampling_x1;
    uint8_t pressureOversampling = oversampling_x1;
    uint8_t deviceModeNormal = 3;
    uint8_t expectedUncommitedRegisterContent =
            (tempOversampling << 5) | (pressureOversampling << 2) | deviceModeNormal;

    saphBme280_prepareMeasureControlReg(&fakeDevice, tempOversampling, pressureOversampling, deviceModeNormal);
    TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
}

void test_saphBme280_prepareMeasureControlReg_checkAllOversamplingModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t amountSamplingOptions = 6;
    uint8_t allOversamplingValues[] = {OVERSAMPLING_SKIP, OVERSAMPLING_x1, OVERSAMPLING_x2, OVERSAMPLING_x4,
                                       OVERSAMPLING_x8, OVERSAMPLING_x16};
    for (uint8_t i = 0; i < amountSamplingOptions; ++i) {
        uint8_t expectedUncommitedRegisterContent =
                allOversamplingValues[i] << 5 | allOversamplingValues[i] << 2 | SAPHBME280_SENSOR_MODE_NORMAL;
        saphBme280_prepareMeasureControlReg(&fakeDevice, allOversamplingValues[i], allOversamplingValues[i],
                                            SAPHBME280_SENSOR_MODE_NORMAL);
        TEST_ASSERT_EQUAL_UINT8(expectedUncommitedRegisterContent, fakeDevice.registerMeasureControl);
    }

}

void test_saphBme280_prepareMeasureControlReg_checkAllSensorModes(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t allSensorModes[] = {SAPHBME280_SENSOR_MODE_SLEEP, SAPHBME280_SENSOR_MODE_FORCED,
                                SAPHBME280_SENSOR_MODE_NORMAL};
    uint8_t amountSensorModes = 3;
    for (int i = 0; i < amountSensorModes; ++i) {
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
    int32_t expectedError = SAPH_BME280_COMM_ERROR_WRITE_AMOUNT;
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

void test_saphBme280_prepareConfigReg_definesStandbyPeriodAndIirFilter(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t standbyTime = SAPHBME280_STANDBY_TIME_MS_1000_0;
    uint8_t iirFilterCoefficient = SAPHBME280_IIR_FILTER_COEFFICIENT_8;
    saphBme280_prepareConfigurationReg(&fakeDevice, standbyTime, iirFilterCoefficient);

    uint8_t expectedRegValue = standbyTime << 5 | iirFilterCoefficient << 1;
    TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
}

void test_saphBme280_prepareConfig_onlyLowestThreeBitsUsedForStandbyTime(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t tooLargeValue = 0xFE;
    uint8_t iirFilterCoefficient = SAPHBME280_IIR_FILTER_COEFFICIENT_8;
    saphBme280_prepareConfigurationReg(&fakeDevice, tooLargeValue, iirFilterCoefficient);

    uint8_t maskedStandbyTimeValue = tooLargeValue & 7;
    uint8_t expectedRegValue = maskedStandbyTimeValue << 5 | iirFilterCoefficient << 1;
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
        uint8_t expectedRegValue = allStandbyTimes[i] << 5 | SAPHBME280_IIR_FILTER_COEFFICIENT_16 << 1;
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
        uint8_t expectedRegValue = SAPHBME280_STANDBY_TIME_MS_125_0 << 5 | allIirCoefficients[i] << 1;
        TEST_ASSERT_EQUAL_UINT8(expectedRegValue, fakeDevice.registerConfig);
    }
}

// #############################################
// # Test group commitConfigRegister
// #############################################

void test_saphBme280_commitConfigRegister_sendsStoredRegisterValue(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    saphBme280_prepareConfigurationReg(&fakeDevice, SAPHBME280_STANDBY_TIME_MS_500_0,
                                       SAPHBME280_IIR_FILTER_COEFFICIENT_2);

    uint8_t configRegisterAddr = 0xF5;
    uint8_t expectedUncommitedRegisterContent =
            SAPHBME280_STANDBY_TIME_MS_500_0 << 5 | SAPHBME280_IIR_FILTER_COEFFICIENT_2 << 1;
    uint8_t expectedBuffer[2] = {configRegisterAddr, expectedUncommitedRegisterContent};
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, expectedBuffer, 2, 2,
                                               2);

    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(SAPH_BME280_NO_ERROR, errorCode);
}

void test_saphBme280_commitConfigRegister_returnsErrorOnWrongAmountWritten(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = SAPH_BME280_COMM_ERROR_WRITE_AMOUNT;
    uint8_t differingReturnValue = 1;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);
    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}

void test_saphBme280_commitConfigRegister_returnsI2cErrorCodeIfGenericHardwareErrorOnWrite(void){
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    int32_t expectedError = ERROR_PLATFORM_GENERIC;
    int32_t differingReturnValue = ERROR_PLATFORM_GENERIC;
    i2c_handler_write_ExpectAnyArgsAndReturn(differingReturnValue);
    int32_t errorCode = saphBme280_commitConfigReg(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(expectedError, errorCode);
}



// #############################################
// # Test group updatePrepareOversampling
// #    for later convenience purposes
// #############################################

//void test_saphBme280_updatePrepareOversampling_changes(void) {
//    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
//
//}
