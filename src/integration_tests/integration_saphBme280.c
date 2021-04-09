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
const uint LED_BLUE_0 = 18;

#define BME_DEFAULT_ADDRESS 0x76

static void init_debug_leds(void);
static void printError(int32_t errorCode, const char* onWhat);
static void integration_runAllTests(saphBmeDevice_t* device);
static void integration_resetDevice(saphBmeDevice_t* device);
static void integration_getId(saphBmeDevice_t* device);
static void integration_getStatus(saphBmeDevice_t* device);
static void integration_setCtrlHumidity(saphBmeDevice_t* device);
static void integration_setMeasureControlReg(saphBmeDevice_t* device);
static void integration_setConfigReg(saphBmeDevice_t* device);
static void integration_getRawMeasurements(saphBmeDevice_t* device);
//static void integration_getPressureValue(saphBmeDevice_t* device);

static void functionalTest(saphBmeDevice_t* device);

int main() {
    stdio_init_all();
    gpio_put(LED_YELLOW_0, 1);
    i2c_handler_initialise(I2C_BAUDRATE);
    init_debug_leds();

    saphBmeDevice_t myBmeDevice = saphBme280_init(BME_DEFAULT_ADDRESS);
    gpio_put(LED_YELLOW_1, 1);
    while (1) {
        integration_runAllTests(&myBmeDevice);
        gpio_put(LED_BLUE_0, 1);
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
//    functionalTest(device);
//    integration_getPressureValue(device);
    printf("################### test done ###################\n\n");
}

static void functionalTest(saphBmeDevice_t* device) {

    saphBme280_resetDevice(device);
    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x4, OVERSAMPLING_x4, SAPHBME280_SENSOR_MODE_SLEEP);
    saphBme280_commitMeasureControlReg(device);
    sleep_ms(10);
    saphBme280_prepareCtrlHumidityReg(device, OVERSAMPLING_x4);
    saphBme280_prepareConfigurationReg(device, SAPHBME280_STANDBY_TIME_MS_10_0, SAPHBME280_IIR_FILTER_COEFFICIENT_2);
    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x4, OVERSAMPLING_x4, SAPHBME280_SENSOR_MODE_NORMAL);
    saphBme280_commitCtrlHumidity(device);
    saphBme280_commitConfigReg(device);
    saphBme280_commitMeasureControlReg(device);
    sleep_ms(100);
    gpio_put(LED_YELLOW_1, 1);
    for (int j = 0; j < 40; ++j) {
        uint8_t bufferSent[] = {0xF2};
        i2c_handler_write(device->address, bufferSent, 1);
        uint8_t bufferReceived[20];
        memset(bufferReceived, 0xAB, 20);
        i2c_handler_read(device->address, bufferReceived, 14);
        printf("hc read: ");
        for (int i = 0; i < 20; ++i) {
            printf("%u,", bufferReceived[i]);
        }

        printf("\n");
        int32_t pressure = (bufferReceived[5] << 11) + (bufferReceived[6] << 3) + (0x07 & bufferReceived[7]);
        int32_t temp = (bufferReceived[8] << 11) + (bufferReceived[9] << 3) + (0x07 & bufferReceived[10]);
        printf("pressure %ld, temp %ld\n", pressure, temp);

    }
    gpio_put(LED_BLUE_0, 1);
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
        printf("status is: measuring %u, im_update %u\n", status & (1 << 3), status & (1 << 0));
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
    }else{
        printf("success\n\n");
    }
}

static void integration_setMeasureControlReg(saphBmeDevice_t* device) {
    printf("integration_setMeasureControlReg: ");
    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x16, OVERSAMPLING_x16, SAPHBME280_SENSOR_MODE_NORMAL);
    int32_t errorCode = saphBme280_commitMeasureControlReg(device);
//    uint8_t bufferSent[] = {0xF4};
//    i2c_handler_write(device->address, bufferSent, 1);
//    uint8_t bufferReceived[20];
//    memset(bufferReceived, 0, 20);
//    i2c_handler_read(device->address, bufferReceived, 1);
//    printf("hc read: ");
//    for (int i = 0; i < 20; ++i) {
//        printf("%u,", bufferReceived[i]);
//    }
    printf("\n");
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

    saphBme280_prepareCtrlHumidityReg(device, OVERSAMPLING_x4);
    errorCode = saphBme280_commitCtrlHumidity(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing ctrlHumidity settings");
        return;
    }

    saphBme280_prepareConfigurationReg(device, SAPHBME280_STANDBY_TIME_MS_0_5, SAPHBME280_IIR_FILTER_COEFFICIENT_OFF);
    errorCode = saphBme280_commitConfigReg(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committing ConfigurationReg settings");
        return;
    }

    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x4, OVERSAMPLING_x4,
                                        SAPHBME280_SENSOR_MODE_SLEEP);
    errorCode = saphBme280_commitMeasureControlReg(device);
    if (errorCode != SAPH_BME280_NO_ERROR) {
        printError(errorCode, "committed MeasureControlReg settings");
        return;
    }

    printf("waiting for measurements to kick in\n");
    sleep_ms(100);

    uint32_t pressure = 0;
    uint32_t temp = 0;
    uint32_t humidity = 0;

    errorCode = saphBme280_getRawMeasurement(device, &pressure, &temp, &humidity);
    if (errorCode == SAPH_BME280_NO_ERROR) {
        printf("successful\n");
        printf("pressure %lu, temp %lu, humidity%lu\n\n", pressure, temp, humidity);
    } else {
        printf("failed\n");
        printf("Error code: %ld\n\n", errorCode);
    }
}

//static void integration_getPressureValue(saphBmeDevice_t* device) {
//    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x1, OVERSAMPLING_x1, SAPHBME280_SENSOR_MODE_SLEEP);
//    int32_t errorCode = saphBme280_commitMeasureControlReg(device);
//    if (errorCode != SAPH_BME280_NO_ERROR) {
//        printError(errorCode, "committing MeasureControlReg settings");
//        return;
//    }
//    printf("integration_getPressureValue: ");
//    saphBme280_prepareConfigurationReg(device, SAPHBME280_STANDBY_TIME_MS_0_5, SAPHBME280_IIR_FILTER_COEFFICIENT_OFF);
//    errorCode = saphBme280_commitConfigReg(device);
//    if (errorCode != SAPH_BME280_NO_ERROR) {
//        printError(errorCode, "committing ConfigurationReg settings");
//        return;
//    }
//    saphBme280_prepareMeasureControlReg(device, OVERSAMPLING_x16, OVERSAMPLING_x16, SAPHBME280_SENSOR_MODE_NORMAL);
//    errorCode = saphBme280_commitMeasureControlReg(device);
//    if (errorCode != SAPH_BME280_NO_ERROR) {
//        printError(errorCode, "committing MeasureControlReg settings");
//        return;
//    }
//    sleep_ms(1000);

//    uint8_t bufferSent[] = {0xF4};
//    i2c_handler_write(device->address, bufferSent, 1);
//    uint8_t bufferReceived[20];
//    memset(bufferReceived, 0, 20);
//    i2c_handler_read(device->address, bufferReceived, 1);
//    printf("hc read: ");
//    for (int i = 0; i < 20; ++i) {
//        printf("%u,", bufferReceived[i]);
//    }
//    printf("\n");
//
//    sleep_ms(1000);
//    uint32_t pressureValues[20];
//
//    for (int i = 0; i < 20; ++i) {
//        saphBme280_getPressure(device, &(pressureValues[i]));
////        printf("%lu, ", pressureValues);
//        sleep_ms(10);
//    }
//    for (int j = 0; j < 20; ++j) {
//        printf("%lu, ", pressureValues[j]);
//    }
//    printf("\n");
//    errorCode = saphBme280_getPressure(device, &pressureValues[4]);
//    if (errorCode != SAPH_BME280_NO_ERROR) {
//        printError(errorCode, "reading pressure value");
//        return;
//    }
//    printf("successful\n");
//    printf("Pressure Value seems to be %lu\n\n", pressureValues[4]);
//}

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
    gpio_init(LED_BLUE_0);
    gpio_set_dir(LED_YELLOW_0, GPIO_OUT);
    gpio_set_dir(LED_YELLOW_1, GPIO_OUT);
    gpio_set_dir(LED_BLUE_0, GPIO_OUT);
    gpio_put(LED_YELLOW_0, 0);
    gpio_put(LED_YELLOW_1, 0);
    gpio_put(LED_BLUE_0, 0);
}

