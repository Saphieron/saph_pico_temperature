#ifndef SAPH_SSD1306_INTERNAL_H
#define SAPH_SSD1306_INTERNAL_H

#include "saph_ssd1306.h"

int32_t saph_ssd1306_internal_sendCtrlCommand(saph_ssd1306_device_t* device, uint8_t* buffer, uint32_t bufferSize);

#endif // SAPH_SSD1306_INTERNAL_H
