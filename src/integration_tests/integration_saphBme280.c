//
// Created by saphieron on 4/2/21.
//

#include <stdio.h>
#include "pico/stdlib.h"

#include "../i2c_handler.h"
#include "../saphBme280.h"

#ifndef I2C_BAUDRATE
#define I2C_BAUDRATE 100000UL
#endif

const uint LED_YELLOW_0 = 16;
const uint LED_YELLOW_1 = 17;
const uint LED_BLUE_0 = 18;

#define BME_DEFAULT_ADDRESS 0x76

void init_debug_leds(void);
void integration_runAllTests(saphBmeDevice_t* device);
void integration_getId(saphBmeDevice_t* device);
void integration_resetDevice(saphBmeDevice_t* device);
void integration_setMeasureControlReg(saphBmeDevice_t* device);
void integration_setConfigReg(saphBmeDevice_t* device);

int main() {
    stdio_init_all();
    i2c_handler_initialise(I2C_BAUDRATE);
//    sleep_ms(5000);
    init_debug_leds();
    gpio_put(LED_YELLOW_0, 1);
    saphBmeDevice_t myBmeDevice = saphBme280_init(BME_DEFAULT_ADDRESS);
    gpio_put(LED_YELLOW_1, 1);

    while (1) {
        integration_runAllTests(&myBmeDevice);
        gpio_put(LED_BLUE_0, 1);
        sleep_ms(1000);
    }
//    return 0;
}

void integration_runAllTests(saphBmeDevice_t* device) {

    printf("##########SaphBme280 Integration tests##########\nRunning all tests:\n");
    integration_getId(device);
    integration_resetDevice(device);
    integration_setMeasureControlReg(device);
    integration_setConfigReg(device);
    printf("################### test done ###################\n\n");
}

void integration_getId(saphBmeDevice_t* device) {
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

void integration_resetDevice(saphBmeDevice_t* device) {
    printf("integration_resetDevice: ");
    int32_t errorCode = saphBme280_resetDevice(device);
    if (errorCode == SAPH_BME280_NO_ERROR) {
        printf("successful\n\n");
    } else {
        printf("failed\n");
        printf("Error code: %ld\n\n", errorCode);
    }
}

void integration_setMeasureControlReg(saphBmeDevice_t* device) {
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

void integration_setConfigReg(saphBmeDevice_t* device) {
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

// Helper functions
void init_debug_leds(void) {
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

