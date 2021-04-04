////
// Created by saphieron on 4/1/21.
//
////
// dependencies of the pico platform
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "i2c_handler.h"
#include <stdio.h>


static i2c_inst_t* selectedI2CInstance = i2c0;

static bool isAddressReserved(uint8_t addr);

uint32_t i2c_handler_initialise(uint32_t baudrate) {
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
//    // Make the I2C pins available to picotool-
//    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    return i2c_init(selectedI2CInstance, baudrate);
}

void i2c_handler_disable(void) {
    i2c_deinit(selectedI2CInstance);
}


int32_t i2c_handler_selectHwInstance(uint8_t device_num) {
    int8_t result = 0;
    switch (device_num) {
        case 0:
            selectedI2CInstance = i2c0;
            break;
        case 1:
            selectedI2CInstance = i2c1;
            break;
        default:
            result = -1;
            break;
    }
    return result;
}

uint32_t i2c_handler_set_baudrate(uint32_t baudrate) {
    return i2c_set_baudrate(selectedI2CInstance, baudrate);
}

int32_t i2c_handler_write(uint8_t addr, uint8_t* buffer, uint32_t amount) {
    return i2c_write_blocking(selectedI2CInstance, addr, buffer, amount, false);
}

int32_t i2c_handler_read(uint8_t addr, uint8_t* buffer, uint32_t amount) {
//    absolute_time_t currentTime = get_absolute_time();
//    return i2c_read_blocking_until(selectedI2CInstance, addr, buffer, amount, false, currentTime + 10000);
    return i2c_read_blocking(selectedI2CInstance, addr, buffer, amount, false);
}

void i2c_handler_scanForDevices(void) {
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    int8_t isCurrentDeviceThere = 0;
    uint8_t buffer;
    for (uint8_t i = 0; i < 128; ++i) {
        if (i % 16 == 0) {
            printf("%02x ", i);
        }
        if (isAddressReserved(i)) {
            isCurrentDeviceThere = PICO_ERROR_GENERIC;
        } else {
            isCurrentDeviceThere = i2c_handler_read(i, &buffer, 1);
        }
        printf(isCurrentDeviceThere < 0 ? "." : "@");
        printf(i % 16 == 15 ? "\n" : "  ");
    }
}

static bool isAddressReserved(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}
