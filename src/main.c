#include <stdio.h>
#include "pico/stdlib.h"


#include "i2c_handler.h"

#ifndef I2C_BAUDRATE
#define I2C_BAUDRATE 100000UL
#endif

const uint LED_YELLOW_0 = 16;
const uint LED_YELLOW_1 = 17;
const uint LED_BLUE_0 = 18;

void init_debug_leds(void);

int main() {
    stdio_init_all();
    init_debug_leds();

    gpio_put(LED_YELLOW_0, 1);
    i2c_handler_initialise(I2C_BAUDRATE);
    gpio_put(LED_YELLOW_1, 1);
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    bool current_val = 0;
    while (1) {
        i2c_handler_scanForDevices();
        gpio_put(LED_BLUE_0, 1);
        gpio_put(LED_PIN, current_val);
        current_val = !current_val;
        printf("another round\n");
        sleep_ms(1000);
    }
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