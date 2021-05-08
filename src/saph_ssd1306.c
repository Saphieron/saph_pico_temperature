#include "saph_ssd1306.h"
#include "saph_ssd1306_internal.h"

void saph_ssd1306_init(uint8_t address, saph_ssd1306_device_t* device) {
    device->address = address;
}

int32_t saph_ssd1306_contrast(saph_ssd1306_device_t* device, uint8_t contrastLevel) {
    if (device == 0) {
        return SAPH_SSD1306_NULL_POINTER_ERROR;
    }
    uint8_t command[2] = {0x81, contrastLevel};
    int32_t errorCode = saph_ssd1306_internal_sendCtrlCommand(device, command, 2);
    return errorCode;
}

#define IGNORE_RAM_CONTENTS 0xA5
#define USE_RAM_CONTENTS 0xA4
int32_t saph_ssd1306_displayOn(saph_ssd1306_device_t* device, bool ignoreRam) {
    if(device == 0){
        return SAPH_SSD1306_NULL_POINTER_ERROR;
    }
    uint8_t command[1] = {USE_RAM_CONTENTS};
    if (ignoreRam) {
        command[0] = IGNORE_RAM_CONTENTS;
    }
    int32_t errorCode = saph_ssd1306_internal_sendCtrlCommand(device, command, 1);
    return errorCode;
}


