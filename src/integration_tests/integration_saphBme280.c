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

void integration_runAllTests(saphBmeDevice_t* device){

    printf("##########SaphBme280 Integration tests##########\nRunning all tests:\n");
    integration_getId(device);
    printf("test done\n\n");
}

void integration_getId(saphBmeDevice_t* device){
    uint8_t id = 0;
    id = saphBme280_getId(device);
    printf("Device id seems to be: 0x%02X\n", id);
}

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

