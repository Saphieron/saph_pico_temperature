#include "saphBme280.h"


saphBmeDevice_t saphBme280_init(uint8_t address){
    saphBmeDevice_t newDevice;
    newDevice.address = address;
    return newDevice;
}