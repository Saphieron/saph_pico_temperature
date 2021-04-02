#include "unity.h"

#include "saphBme280.h"

#include "mock_i2c_handler.h"

void setUp(void) {
}

void tearDown(void) {
}

/* *
 * TODO: Check notes, cross out what you have
 * Notes:
 *  - Sensor has a mode, allow it to be set. sleep mode, normal mode, forced mode
 *  - This might mean i need to allow for a i2cwrite to finish on a repeated start or no stop
 *
 * Steps for operating it:
 *  - The sensor 'boots' in sleep mode. Set mode[1:0] to 11 i.e. normal so that it starts? or 01 for a single measurement
 *  - After that:
 *  - Writing to a bme register is: slave address, the register address, and the register value. The content i need to write is therefore only the register addr and its value before sending a final stop.
 *  - Reading is: Write register, send stop, then read n bytes until you have all subsequent registers read. send stop to finish
 *
 * Configurations:
 *      -
 *
 * */

void test_saphBme280_initialises(void){
    uint8_t deviceAddr = 0xAB;
    saphBmeDevice_t bmeDevice = saphBme280_init(deviceAddr);
    TEST_ASSERT_EQUAL_UINT8(deviceAddr, bmeDevice.address);
}

void test_saphBme280_getId(void){
    uint8_t deviceAddr = 0xAB;
    saphBmeDevice_t bmeDevice = saphBme280_init(deviceAddr);

}


