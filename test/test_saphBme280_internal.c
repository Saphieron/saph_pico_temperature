#include "unity.h"

#include "saphBme280.h"
#include "saphBme280_internal.h"

#include "mock_i2c_handler.h"

static saphBmeDevice_t helper_createBmeDevice(void) {
    uint8_t deviceAddr = 0xF7;
    saphBmeDevice_t bmeDevice = {deviceAddr};
    return bmeDevice;
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

    saphBmeMeasurements_t result = saphBme280_compensateMeasurements(&fakeDevice, &rawMeasurements);
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

    saphBmeMeasurements_t result = saphBme280_compensateMeasurements(&fakeDevice, &rawMeasurements);
    TEST_ASSERT_EQUAL_UINT32(expectedPressure, result.pressure);
}

int32_t helper_calculateHumidity(saphBmeDevice_t* fakeDevice, saphBmeRawMeasurements_t* rawMeasurements) {
    TEST_FAIL_MESSAGE("not implemented properly yet");
    return -1;
}

void test_saphBme280_compensateMeasurements_compensateHumidityReading(void) {
    saphBmeDevice_t fakeDevice = helper_createBmeDevice();
    helper_setTrimmingValues(&fakeDevice);
    saphBmeRawMeasurements_t rawMeasurements = {283413, 523407, 27999};
    int32_t expectedHumidity = helper_calculateHumidity(&fakeDevice, &rawMeasurements);

    saphBmeMeasurements_t result = saphBme280_compensateMeasurements(&fakeDevice, &rawMeasurements);
    TEST_ASSERT_EQUAL_UINT32(expectedHumidity, result.pressure);
}
