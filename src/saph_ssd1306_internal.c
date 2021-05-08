#include "saph_ssd1306_internal.h"

#include "i2c_handler.h"
#include <string.h>

int32_t saph_ssd1306_internal_sendCtrlCommand(saph_ssd1306_device_t* device, uint8_t* buffer, uint32_t bufferSize) {
    if (device == 0 || buffer == 0) {
        return SAPH_SSD1306_NULL_POINTER_ERROR;
    }
    uint8_t sendingBuffer[bufferSize + 1];
    sendingBuffer[0] = 0x00;
    memcpy(sendingBuffer + 1, buffer, bufferSize);
    int32_t errorCode = i2c_handler_write(device->address, sendingBuffer, bufferSize + 1);
    return errorCode;
}
