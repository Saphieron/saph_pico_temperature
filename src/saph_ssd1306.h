#ifndef SAPH_SSD1306_H
#define SAPH_SSD1306_H

#include <stdint.h>
#include <stdbool.h>

#define SAPH_SSD1306_NO_ERROR 0
#define SAPH_SSD1306_COMM_ERROR_WRITE_AMOUNT -11
#define SAPH_SSD1306_COMM_ERROR_READ_AMOUNT -12
#define SAPH_SSD1306_NULL_POINTER_ERROR -20
#define SAPH_SSD1306_RESERVED_ADDR_ERROR -30

typedef struct saph_ssd1306_device_t {
    uint8_t address;
} saph_ssd1306_device_t;

void saph_ssd1306_init(uint8_t address, saph_ssd1306_device_t* device);

int32_t saph_ssd1306_contrast(saph_ssd1306_device_t* device, uint8_t contrastLevel);

int32_t saph_ssd1306_displayOn(saph_ssd1306_device_t* device, bool ignoreRam);

#endif // SAPH_SSD1306_H
