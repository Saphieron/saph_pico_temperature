#include "unity.h"

#include "saphBme280.h"
#include "saphBme280_internal.h"
#include "test_saphBme280_test_definitions.h"

#include "mock_i2c_handler.h"

static saphBmeDevice_t helper_createBmeDevice(void) {
    uint8_t deviceAddr = 0xF7;
    saphBmeDevice_t bmeDevice = {deviceAddr};
    return bmeDevice;
}

// #############################################
// # Test group _readTrimmingValues
// #############################################

uint8_t trimmingFirstResponse[firstBurstReadAmount];
uint8_t trimmingSecondResponse[secondBurstReadAmount];
uint8_t trimmingThirdResponse[thirdBurstReadAmount];

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
    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_checkTemperatureTrimmingValues(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    saphBme280_internal_readTrimmingValues(&fakeDevice);
    saphBmeTrimmingValues_t trimmingValues = fakeDevice.trimmingValues;
    helper_checkUnsignedTrimmingValue(trimmingFirstResponse, &(trimmingValues.dig_T1));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 2, &(trimmingValues.dig_T2));
    helper_checkSignedTrimmingValue(trimmingFirstResponse + 4, &(trimmingValues.dig_T3));
}

void test_saphBme280_readTrimmingValues_checkPressureTrimmingValues(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    saphBme280_internal_readTrimmingValues(&fakeDevice);
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

void test_saphBme280_readTrimmingValues_checkHumidityTrimmingValues(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_prepareI2cBurstRead(&fakeDevice);
    saphBme280_internal_readTrimmingValues(&fakeDevice);

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

    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedFirstRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    int32_t wrongReadAmount = 0;
    i2c_handler_read_ExpectAnyArgsAndReturn(wrongReadAmount);

    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedSecondWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(25);
    int32_t wrongWriteAmount = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(wrongWriteAmount);
    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, errorCode);
}

void test_saphBme280_readTrimmingValues_returnsErrorCodeOnFailedSecondRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(25);
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    int32_t wrongReadAmount = 0;
    i2c_handler_read_ExpectAnyArgsAndReturn(wrongReadAmount);
    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
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
    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
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
    int32_t errorCode = saphBme280_internal_readTrimmingValues(&fakeDevice);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, errorCode);
}

// #############################################
// # Test group _getRawAllMeasurements
// #############################################

void test_saphBme280_getRawAllMeasurements_returnsPressureThroughStruct(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t startingRegister = 0xF7; // the pressure register
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &startingRegister, 1, 1, 1);
    uint8_t response[] = {0xFF, 0x00, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int32_t expectedPressure = (response[0] << 12) + (response[1] << 4) + (response[2] >> 4);
    saphBmeRawMeasurements_t result;
    result.pressure = 0;

    i2c_handler_read_ExpectAnyArgsAndReturn(MEASUREMENT_SIZE);
    i2c_handler_read_ReturnArrayThruPtr_buffer(response, MEASUREMENT_SIZE);
    int32_t errorCode = saphBme280_internal_getRawMeasurement(&fakeDevice, &result);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_INT32(expectedPressure, result.pressure);
}

void test_saphBme280_getRawAllMeasurements_returnsTemperatureThroughStruct(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    uint8_t startingRegister = 0xF7; // the pressure register
    i2c_handler_write_ExpectWithArrayAndReturn(fakeDevice.address, &startingRegister, 1, 1, 1);
    uint8_t response[] = {0x00, 0x00, 0x00, 0xFF, 0x00, 0xD0, 0x00, 0x00};
    int32_t expectedTemp = (response[3] << 12) + (response[4] << 4) + (response[5] >> 4);
    saphBmeRawMeasurements_t result;
    result.temperature = 0;

    i2c_handler_read_ExpectAnyArgsAndReturn(MEASUREMENT_SIZE);
    i2c_handler_read_ReturnArrayThruPtr_buffer(response, MEASUREMENT_SIZE);
    int32_t errorCode = saphBme280_internal_getRawMeasurement(&fakeDevice, &result);
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
    int32_t errorCode = saphBme280_internal_getRawMeasurement(&fakeDevice, &result);
    TEST_ASSERT_EQUAL_INT32(NO_ERROR, errorCode);
    TEST_ASSERT_EQUAL_INT32(expectedHumidity, result.humidity);
}

void test_saphBme280_getRawAllMeasurements_returnsErrorCodeForFailedWrite(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    void* nothingness = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(0);
    int32_t result = saphBme280_internal_getRawMeasurement(&fakeDevice, nothingness);
    TEST_ASSERT_EQUAL_INT32(WRITE_ERROR, result);
}

void test_saphBme280_getRawAllMeasurements_returnsErrorCodeForFailedRead(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    void* nothingness = 0;
    i2c_handler_write_ExpectAnyArgsAndReturn(1);
    i2c_handler_read_ExpectAnyArgsAndReturn(1);
    int32_t result = saphBme280_internal_getRawMeasurement(&fakeDevice, nothingness);
    TEST_ASSERT_EQUAL_INT32(READ_ERROR, result);
}


// #############################################
// # Test group _compensateMeasurements
// #############################################

// Trimming values from my personal bme280
// T1 28417, T2 26721, T3 50
// P1 38042, P2 -10559, P3 3024, P4 8726, P5 -185, P6 -7, P7 9900, P8 -10230, P9 4285
// H1 75, H2 360, H3 0, H4 325, H5 50

void helper_setTrimmingValues(saphBmeDevice_t* fakeDevice) {
    saphBmeTrimmingValues_t notSoFakeTrimValues = {28417, 26721, 50,
                                                   38042, -10559, 3024,
                                                   8726, -185, -7,
                                                   9900, -10230, 4285,
                                                   75, 360, 0,
                                                   325, 50};
    fakeDevice->trimmingValues = notSoFakeTrimValues;
}

void test_saphBme280_compensateMeasurement_compensatesTemperatureReading(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_setTrimmingValues(&fakeDevice);
    saphBmeRawMeasurements_t rawMeasurements = {283413, 523407, 27999};
//    saphBmeTrimmingValues_t trims = fakeDevice.trimmingValues;
//    int32_t fineTemperature;
//    int32_t rawTemp = rawMeasurements.temperature;
//    int32_t value1 = (((rawTemp >> 3) - ((int32_t) trims.dig_T1 << 1)) * ((int32_t) trims.dig_T2)) >> 11;
//    int32_t value2 = ((rawTemp >> 4) - ((int32_t) trims.dig_T1));
//    value2 = (value2 * value2) >> 12;
//    value2 *= ((int32_t) trims.dig_T3);
//    value2 = value2 >> 14;
//    fineTemperature = value1 + value2;
//    int32_t temp = (fineTemperature * 5 + 128) >> 8;
    int32_t expectedTemperature = 2189;

    saphBmeMeasurements_t result = saphBme280_internal_compensateMeasurements(&fakeDevice, &rawMeasurements);
    TEST_ASSERT_EQUAL_INT32(expectedTemperature, result.temperature);
}

int32_t helper_calculateFineTemperature(saphBmeDevice_t* fakeDevice, saphBmeRawMeasurements_t* rawMeasurements) {
    saphBmeTrimmingValues_t trims = fakeDevice->trimmingValues;
    int32_t rawTemp = rawMeasurements->temperature;
    int32_t value1 = (((rawTemp >> 3) - ((int32_t) trims.dig_T1 << 1)) * ((int32_t) trims.dig_T2)) >> 11;
    int32_t value2 = ((rawTemp >> 4) - ((int32_t) trims.dig_T1));
    value2 = (value2 * value2) >> 12;
    value2 *= ((int32_t) trims.dig_T3);
    value2 = value2 >> 14;
    int32_t fineTemperature = value1 + value2;
    return fineTemperature;
}

uint32_t helper_calculatePressure(saphBmeDevice_t* fakeDevice, saphBmeRawMeasurements_t* rawMeasurements) {
    saphBmeTrimmingValues_t trims = fakeDevice->trimmingValues;
    int32_t fineTemperature = helper_calculateFineTemperature(fakeDevice, rawMeasurements);
    int64_t value1, value2, pressure;
    value1 = ((int64_t) fineTemperature) - 128000;
    value2 = value1 * value1 * ((int64_t) trims.dig_P6);
    value2 = value2 + ((value1 * ((int64_t) trims.dig_P5)) << 17);
    value2 = value2 + (((int64_t) trims.dig_P4) << 35);
    value1 = ((value1 * value1 * (int64_t) trims.dig_P3) >> 8) + ((value1 * (int64_t) trims.dig_P2) << 12);
    value1 = (((((int64_t) 1) << 47) + value1) * ((int64_t) trims.dig_P1)) >> 33;
    if (value1 == 0) {
        TEST_FAIL_MESSAGE("value1 was 0");
    }
    pressure = 1048576 - rawMeasurements->pressure;
    pressure = (((pressure << 31) - value2) * 3125) / value1;
    value1 = (((int64_t) trims.dig_P9) * (pressure >> 13) * (pressure > 13)) >> 25;
    value2 = (((int64_t) trims.dig_P8) * pressure) >> 19;
    pressure = ((pressure + value1 + value2) >> 8) + (((int64_t) trims.dig_P7) << 4);
    uint32_t result = (uint32_t) pressure;
    return result;
}

void test_saphBme280_compensateMeasurements_compensatePressureReading(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_setTrimmingValues(&fakeDevice);
    saphBmeRawMeasurements_t rawMeasurements = {283413, 523407, 27999};
    uint32_t expectedPressure = helper_calculatePressure(&fakeDevice, &rawMeasurements);

    saphBmeMeasurements_t result = saphBme280_internal_compensateMeasurements(&fakeDevice, &rawMeasurements);
    TEST_ASSERT_EQUAL_UINT32(expectedPressure, result.pressure);
}

uint32_t helper_calculateHumidity(saphBmeDevice_t* fakeDevice, saphBmeRawMeasurements_t* rawMeasurements) {
    int32_t rawHumidity = rawMeasurements->humidity;
    saphBmeTrimmingValues_t trims = fakeDevice->trimmingValues;
    int32_t variable; //I don't know what the datasheet was trying to tell me with their og name
    int32_t fineTemperature = helper_calculateFineTemperature(fakeDevice, rawMeasurements);
    variable = (fineTemperature - ((int32_t) 76800));
    variable =
            ((((rawHumidity << 14) - (((int32_t) trims.dig_H4) << 20) -
               (((int32_t) trims.dig_H5) * variable)) + ((int32_t) 16348)) >> 15) *
            (((((((variable * ((int32_t) trims.dig_H6) >> 10) * (variable * ((int32_t) trims.dig_H3)) >> 11) +
                 ((int32_t) 32768)) >> 10) + ((int32_t) 2097152)) * ((int32_t) trims.dig_H2) + 8192) >> 14);
    variable = (variable - (((((variable >> 15) * (variable >> 15)) >> 7) * ((int32_t) trims.dig_H1)) >> 4));
    variable = (variable < 0 ? 0 : variable);
    return (uint32_t) (variable >> 12);
}

void test_saphBme280_compensateMeasurements_compensateHumidityReading(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_setTrimmingValues(&fakeDevice);
    saphBmeRawMeasurements_t rawMeasurements = {283413, 523407, 27999};
    int32_t expectedHumidity = helper_calculateHumidity(&fakeDevice, &rawMeasurements);

    saphBmeMeasurements_t result = saphBme280_internal_compensateMeasurements(&fakeDevice, &rawMeasurements);
    TEST_ASSERT_EQUAL_UINT32(expectedHumidity, result.humidity);
}
