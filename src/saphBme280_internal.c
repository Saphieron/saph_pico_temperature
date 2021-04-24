#include "saphBme280_internal.h"
#include "saphBme280.h"

typedef struct tempResults_t {
    int32_t fineTemperature;
    int32_t temperature;
} tempResults_t;

// ###############################################
// Helper Functions defined
// ###############################################
static tempResults_t compensateTemperature(saphBmeDevice_t* device, int32_t rawTemperature);

static uint32_t compensatePressure(saphBmeDevice_t* device, int32_t rawPressure, int32_t fineTemperature);

// ###############################################
// Implementations
// ###############################################

saphBmeMeasurements_t
saphBme280_compensateMeasurements(saphBmeDevice_t* device, saphBmeRawMeasurements_t* rawMeasurements) {
    saphBmeMeasurements_t result;
    tempResults_t temps = compensateTemperature(device, rawMeasurements->temperature);
    result.temperature = temps.temperature;
    result.pressure = compensatePressure(device, rawMeasurements->pressure, temps.fineTemperature);
    return result;
}

// ###############################################
// Helper Functions
// ###############################################

static tempResults_t compensateTemperature(saphBmeDevice_t* device, int32_t rawTemperature) {
    saphBmeTrimmingValues_t trimmingVals = device->trimmingValues;
    int32_t value1, value2;
    value1 = (rawTemperature >> 3) - (((int32_t) trimmingVals.dig_T1) << 1);
    value1 = (value1 * ((int32_t) trimmingVals.dig_T2)) >> 11;
    value2 = (rawTemperature >> 4) - ((int32_t) trimmingVals.dig_T1);
    value2 = (((value2 * value2) >> 12) * ((int32_t) trimmingVals.dig_T3)) >> 14;
    tempResults_t result;
    result.fineTemperature = value1 + value2;
    result.temperature = (result.fineTemperature * 5 + 128) >> 8;
    return result;
}

static uint32_t compensatePressure(saphBmeDevice_t* device, int32_t rawPressure, int32_t fineTemperature) {
    saphBmeTrimmingValues_t trims = device->trimmingValues;
    int64_t value1, value2, pressure;
    value1 = ((int64_t) fineTemperature) - 128000;
    value2 = value1 * value1 * ((int64_t) trims.dig_P6);
    value2 = value2 + ((value1 * ((int64_t) trims.dig_P5)) << 17);
    value2 = value2 + (((int64_t) trims.dig_P4) << 35);
    value1 = ((value1 * value1 * (int64_t) trims.dig_P3) >> 8) + ((value1 * (int64_t) trims.dig_P2) << 12);
    value1 = (((((int64_t) 1) << 47) + value1) * ((int64_t) trims.dig_P1)) >> 33;
    if (value1 == 0) {
        return 0;
    }
    pressure = 1048576 - rawPressure;
    pressure = (((pressure << 31) - value2) * 3125) / value1;
    value1 = (((int64_t) trims.dig_P9) * (pressure >> 13) * (pressure > 13)) >> 25;
    value2 = (((int64_t) trims.dig_P8) * pressure) >> 19;
    pressure = ((pressure + value1 + value2) >> 8) + (((int64_t) trims.dig_P7) << 4);
    uint32_t result = (uint32_t) pressure;
    return result;
}