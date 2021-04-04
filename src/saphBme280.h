#include <sched.h>

#ifndef SAPHBME280_H
#define SAPHBME280_H

#include <stdint.h>

typedef struct saphBmeDevice_t {
    uint8_t address;
    uint8_t registerMeasureControl;
    uint8_t registerConfig;
} saphBmeDevice_t;

#define SAPH_BME280_NO_ERROR 0
#define SAPH_BME280_COMM_ERROR_WRITE_AMOUNT -11
#define SAPH_BME280_COMM_ERROR_READ_AMOUNT -12


#define OVERSAMPLING_SKIP 0x00
#define OVERSAMPLING_x1 0x01
#define OVERSAMPLING_x2 0x02
#define OVERSAMPLING_x4 0x03
#define OVERSAMPLING_x8 0x04
#define OVERSAMPLING_x16 0x05

#define SAPHBME280_SENSOR_MODE_SLEEP 2
#define SAPHBME280_SENSOR_MODE_FORCED 2
#define SAPHBME280_SENSOR_MODE_NORMAL 3

#define SAPHBME280_STANDBY_TIME_MS_0_5 0x00
#define SAPHBME280_STANDBY_TIME_MS_62_5 0x01
#define SAPHBME280_STANDBY_TIME_MS_125_0 0x02
#define SAPHBME280_STANDBY_TIME_MS_250_0 0x03
#define SAPHBME280_STANDBY_TIME_MS_500_0 0x04
#define SAPHBME280_STANDBY_TIME_MS_1000_0 0x05
#define SAPHBME280_STANDBY_TIME_MS_10_0 0x06
#define SAPHBME280_STANDBY_TIME_MS_20_0 0x07

#define SAPHBME280_IIR_FILTER_COEFFICIENT_OFF 0x00
#define SAPHBME280_IIR_FILTER_COEFFICIENT_2 0x01
#define SAPHBME280_IIR_FILTER_COEFFICIENT_4 0x02
#define SAPHBME280_IIR_FILTER_COEFFICIENT_8 0x03
#define SAPHBME280_IIR_FILTER_COEFFICIENT_16 0x04

saphBmeDevice_t saphBme280_init(uint8_t address);

int32_t saphBme280_getId(saphBmeDevice_t* device);

int32_t saphBme280_resetDevice(saphBmeDevice_t* device);

void
saphBme280_prepareMeasureControlReg(saphBmeDevice_t* device, uint8_t tempOversampling, uint8_t pressureOversampling,
                                    uint8_t sensorMode);

int32_t saphBme280_commitMeasureControlReg(saphBmeDevice_t* device);

void saphBme280_prepareConfigurationReg(saphBmeDevice_t* device, uint8_t standbyTimeInNormalMode,
                                        uint8_t iirFilterCoefficient);

int32_t saphBme280_commitConfigReg(saphBmeDevice_t* device);

#endif // SAPHBME280_H
