//
// Created by saphieron on 4/2/21.
//

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "../i2c_handler.h"
#include "../saphBme280.h"

#ifndef I2C_BAUDRATE
#define I2C_BAUDRATE 400000UL
#endif

const uint LED_YELLOW_0 = 16;
const uint LED_YELLOW_1 = 17;
const uint LED_YELLOW_2 = 18;

#define BME_DEFAULT_ADDRESS 0x76

static void printError(int32_t errorCode, const char* onWhat);

static void integration_runAllTests(saphBmeDevice_t* device);

static void integration_resetDevice(saphBmeDevice_t* device);

static void integration_getId(saphBmeDevice_t* device);

static void integration_getStatus(saphBmeDevice_t* device);

static void integration_setCtrlHumidity(saphBmeDevice_t* device);

static void integration_setMeasureControlReg(saphBmeDevice_t* device);

static void integration_setConfigReg(saphBmeDevice_t* device);

static void integration_getRawMeasurements(saphBmeDevice_t* device);

static void integration_getTrimmingValues(saphBmeDevice_t* device);

//static void integration_getPressureValue(saphBmeDevice_t* device);

// ######################################
// helper functions
// ###################§##################
static void init_debug_leds(void);

static int32_t configIntoKnownSensorState(saphBmeDevice_t* device);

//static void integration_useAllFunctions(saphBmeDevice_t* device);

int main() {
    stdio_init_all();
    init_debug_leds();
    gpio_put(LED_YELLOW_0, 1);
    i2c_handler_initialise(I2C_BAUDRATE);

    saphBmeDevice_t myBmeDevice = saphBme280_init(BME_DEFAULT_ADDRESS);
    gpio_put(LED_YELLOW_1, 1);
    while (1) {
        integration_runAllTests(&myBmeDevice);
        gpio_put(LED_YELLOW_2, 1);
        sleep_ms(5000);
    }
//    return 0;
}

static void integration_runAllTests(saphBmeDevice_t* device) {
// TODO: rewrite test so that it waits an appropriate time after initialisation to read out measurements. Else there is only garbage read out
// TODO: Or just perform multiple and show that it works through that
    printf("##########SaphBme280 Integration tests##########\nRunning all tests:\n");
    integration_resetDevice(device);
    integration_getId(device);
    integration_setCtrlHumidity(device);
    integration_setConfigReg(device);
    integration_setMeasureControlReg(device);
    integration_getStatus(device);
    integration_getRawMeasurements(device);
    integration_getTrimmingValues(device);
//    functionalTest(device);
//    integration_getPressureValue(device);
//    integration_useAllFunctions(device);
    printf("################### test done ###################\n\n");
}

static void integration_useAllFunctions(saphBmeDevice_t* device) {
    printf("integration_useAllFunctions: ");
    saphBme280_resetDevice(device);
    sleep_ms(10);
    configIntoKnownSensorState(device);
    sleep_ms(10);
    gpio_put(LED_YELLOW_1, 1);
    for (int j = 0; j < 40; ++j) {
        uint8_t bufferSent[] = {0xF2};
        i2c_handler_write(device->address, bufferSent, 1);
        uint8_t bufferReceived[12];
        memset(bufferReceived, 0xAB, 12);
        i2c_handler_read(device->address, bufferReceived, 12);
        printf("raw read: ");
        for (int i = 0; i < 12; ++i) {
            printf("%u,", bufferReceived[i]);
        }

        printf("\n");
        uint32_t pressure = (bufferReceived[4] << 11) + (bufferReceived[5] << 3) + (0x07 & bufferReceived[6]);
        uint32_t temp = (bufferReceived[7] << 11) + (bufferReceived[8] << 3) + (0x07 & bufferReceived[9]);
        uint16_t humidity = (bufferReceived[10] << 8) + (bufferReceived[11]);
        printf("pressure %lu, temp %lu, humidity %u\n", pressure, temp, humidity);

        sleep_ms(100);
    }
    gpio_put(LED_YELLOW_2, 1);
}

static void integration_resetDevice(saphBmeDevice_t* device) {
    printf("integration_resetDevice: ");
    int32_t errorCode = saphBme280_resetDevice(device);
    if (errorCode == SAPH_BME280_NO_ERROR) {
        printf("successful\n\n");
    } else {
        printf("failed\n");
        printf("Error code: %ld\n\n", errorCode);
    }
}

static void integration_getId(saphBmeDevice_t* device) {
    int32_t id = 0;
    printf("integration_getId: ");
    id = saphBme280_getId(device);
    if (id < 0) {
        printf("failed\n");
        printf("error code: %ld\n\n", id);
    } else {
        printf("successful\n");
        printf("Device id seems to be: 0x%02X\n\n", (uint8_t) id);
    }
}

static void integration_getStatus(saphBmeDevice_t* device) {
    printf("integration_getStatus: ");
    uint8_t status = 0xFF;
    int32_t errorCode = saphBme280_status(device, &status);
    if (errorCode == SAPH_BME280_NO_ERROR) {
        printf("successful\n");
        printf("status is: measuring %u, im_update %u\n\n", status & (1 << 3), status & (1 << 0));
    } else {
        printf("failed\n");
        printf("Error code: %ld\n\n", errorCode);
    }
}

static void integration_setCtrlHumidity(saphBmeDevice_t* device) {
    printf("integration_setMeasureControlReg: ");
    saphBme280_prepareCtrlHumidityReg(device, OVERSAMPLING_x4);
    int32_t errorCode = saphBme280_commitCtrlHumidity(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing ctrl humidity value");
    } else {
        printf("success\n\n");
    }
    sleep_ms(100);
    uint8_t bufferSent[] = {0xF2};
    i2c_handler_write(device->address, bufferSent, 1);
    uint8_t bufferReceived[12];
    memset(bufferReceived, 0xAB, 12);
    i2c_handler_read(device->address, bufferReceived, 1);
    printf("reading all: ");
    for (int i = 0; i < 1; ++i) {
        printf("%u,", bufferReceived[i]);
    }
    printf("\n");
}

static void integration_setMeasureControlReg(saphBmeDevice_t* device) {
    printf("integration_setMeasureControlReg: ");
    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x16, OVERSAMPLING_x16, SAPHBME280_SENSOR_MODE_NORMAL);
    int32_t errorCode = saphBme280_commitMeasureControlReg(device);

    if (errorCode == SAPH_BME280_NO_ERROR) {
        printf("successful\n\n");
    } else {
        printf("failed\n");
        printf("Error code: %ld\n\n", errorCode);
    }
}

static void integration_setConfigReg(saphBmeDevice_t* device) {
    printf("integration_setConfigReg: ");
    saphBme280_prepareConfigurationReg(device, SAPHBME280_STANDBY_TIME_MS_0_5, SAPHBME280_IIR_FILTER_COEFFICIENT_16);
    int32_t errorCode = saphBme280_commitConfigReg(device);
    if (errorCode == SAPH_BME280_NO_ERROR) {
        printf("successful\n\n");
    } else {
        printf("failed\n");
        printf("Error code: %ld\n\n", errorCode);
    }
}

static void integration_getRawMeasurements(saphBmeDevice_t* device) {
    printf("integration_getRawMeasurements: ");
    // preparation
    int32_t errorCode = saphBme280_resetDevice(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "resetting device");
        return;
    }
    if (configIntoKnownSensorState(device) != SAPH_BME280_NO_ERROR) {
        return;
    }
    sleep_ms(100);
//
//    uint8_t bufferSent[] = {0xF2};
//    i2c_handler_write(device->address, bufferSent, 1);
//    uint8_t bufferReceived[12];
//    memset(bufferReceived, 0xAB, 12);
//    i2c_handler_read(device->address, bufferReceived, 12);
//    printf("reading all: ");
//    for (int i = 0; i < 12; ++i) {
//        printf("%u,", bufferReceived[i]);
//    }

    printf("\nwaiting for measurements to kick in...\n");

    saphBmeMeasurements_t result[20];
    for (int j = 0; j < 20; ++j) {
        errorCode = saphBme280_getRawMeasurement(device, &result[j]);
        if (errorCode != SAPH_BME280_NO_ERROR) {
            printError(errorCode, "getting raw measurement");
        }
        sleep_ms(10);
    }
    for (int k = 0; k < 20; ++k) {
        printf("pressure %lu, temp %lu, humidity%u\n", result[k].pressure, result[k].temperature, result[k].humidity);
    }
//
//    errorCode = saphBme280_getRawMeasurement(device, &result);
//    if (errorCode != SAPH_BME280_NO_ERROR) {
//        printError(errorCode, "getting raw measurement");
//    } else {
//        printf("successful\n");
//        printf("pressure %lu, temp %lu, humidity%u\n\n", result.pressure, result.temperature, result.humidity);
//    }
}

static void integration_getPressureValue(saphBmeDevice_t* device) {
    printf("integration_getPressureValue: ");
    if (configIntoKnownSensorState(device) != SAPH_BME280_NO_ERROR) {
        return;
    }

    uint32_t pressureValues[20];
    int32_t errorCode;
    for (int i = 0; i < 20; ++i) {
        errorCode = saphBme280_getPressure(device, &(pressureValues[i]));
        if (errorCode != SAPH_BME280_NO_ERROR) {
            printError(errorCode, "reading pressure value");
            return;
        }
        sleep_ms(1);
    }
    printf("successful\npressure values: ");
    for (int j = 0; j < 20; ++j) {
        printf("%lu, ", pressureValues[j]);
    }
    printf("\n\n");
}

static void integration_getTrimmingValues(saphBmeDevice_t* device){
    int32_t errorCode = saphBme280_readTrimmingValues(device);
    if(errorCode != SAPH_BME280_NO_ERROR){
        printError(errorCode, "reading Trimming values");
    }
    printf("successful\ntrimmingValues: ");
    saphBmeTrimmingValues_t trimmingVals = device->trimmingValues;
    printf("%u, ", trimmingVals.dig_T1);
    printf("%d, ", trimmingVals.dig_T2);
    printf("%d - ", trimmingVals.dig_T3);

    printf("%u, ", trimmingVals.dig_P1);
    printf("%d, ", trimmingVals.dig_P2);
    printf("%d, ", trimmingVals.dig_P3);
    printf("%d, ", trimmingVals.dig_P4);
    printf("%d, ", trimmingVals.dig_P5);
    printf("%d, ", trimmingVals.dig_P6);
    printf("%d, ", trimmingVals.dig_P7);
    printf("%d, ", trimmingVals.dig_P8);
    printf("%d - ", trimmingVals.dig_P9);

    printf("%u, ", trimmingVals.dig_H1);
    printf("%d, ", trimmingVals.dig_H2);
    printf("%u, ", trimmingVals.dig_H3);
    printf("%d, ", trimmingVals.dig_H4);
    printf("%d\n\n", trimmingVals.dig_H5);
}

// ############################
// # Helpers
// ############################
static void printError(int32_t errorCode, const char* onWhat) {
    printf("failed\n");
    printf("Step that failed: %s\n", onWhat);
    printf("Error code: %ld\n\n", errorCode);
}

// Helper functions
static void init_debug_leds(void) {
    // Preparing leds for later debugging
    gpio_init(LED_YELLOW_0);
    gpio_init(LED_YELLOW_1);
    gpio_init(LED_YELLOW_2);
    gpio_set_dir(LED_YELLOW_0, GPIO_OUT);
    gpio_set_dir(LED_YELLOW_1, GPIO_OUT);
    gpio_set_dir(LED_YELLOW_2, GPIO_OUT);
    gpio_put(LED_YELLOW_0, 0);
    gpio_put(LED_YELLOW_1, 0);
    gpio_put(LED_YELLOW_2, 0);
}

static int32_t configIntoKnownSensorState(saphBmeDevice_t* device) {
    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x1, OVERSAMPLING_x1, SAPHBME280_SENSOR_MODE_SLEEP);
    int32_t errorCode = saphBme280_commitMeasureControlReg(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing MeasureControlReg settings");
        return errorCode;
    }

    saphBme280_prepareCtrlHumidityReg(device, OVERSAMPLING_x1);
    errorCode = saphBme280_commitCtrlHumidity(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing ctrlHumidity settings");
        return errorCode;
    }

    saphBme280_prepareConfigurationReg(device, SAPHBME280_STANDBY_TIME_MS_0_5, SAPHBME280_IIR_FILTER_COEFFICIENT_2);
    errorCode = saphBme280_commitConfigReg(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing ConfigurationReg settings");
        return errorCode;
    }
    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x1, OVERSAMPLING_x1, SAPHBME280_SENSOR_MODE_NORMAL);
    errorCode = saphBme280_commitMeasureControlReg(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing MeasureControlReg settings");
        return errorCode;
    }
    sleep_ms(10);
    return SAPH_BME280_NO_ERROR;
}

