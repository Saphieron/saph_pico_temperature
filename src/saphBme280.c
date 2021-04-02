#include "saphBme280.h"
#include "i2c_handler.h"


saphBmeDevice_t saphBme280_init(uint8_t address){
    saphBmeDevice_t newDevice;
    newDevice.address = address;
    return newDevice;
}

uint8_t saphBme280_getId(saphBmeDevice_t* device){
    uint8_t  idRegister = 0xD0;
    i2c_handler_write(device->address, &idRegister, 1);
    uint8_t id = 0;
    i2c_handler_read(device->address, &id, 1);
    return id;
}