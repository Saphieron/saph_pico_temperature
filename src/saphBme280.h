#ifndef SAPHBME280_H
#define SAPHBME280_H

#include <stdint.h>

typedef struct saphBmeTrimmingValues_t {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} saphBmeTrimmingValues_t;

typedef struct saphBmeDevice_t {
    uint8_t address;
    uint8_t registerCtrlHumidity;
    uint8_t registerMeasureCtrl;
    uint8_t registerConfig;
    saphBmeTrimmingValues_t trimmingValues;
} saphBmeDevice_t;

typedef struct saphBmeMeasurements_t {
    uint32_t pressure;
    int32_t temperature;
    uint32_t humidity;
} saphBmeMeasurements_t;

#define SAPH_BME280_NO_ERROR 0
#define SAPH_BME280_COMM_ERROR_WRITE_AMOUNT -11
#define SAPH_BME280_COMM_ERROR_READ_AMOUNT -12
#define SAPH_BME280_NULL_POINTER_ERROR -20
#define SAPH_BME280_RESERVED_ADDR_ERROR -30

#define OVERSAMPLING_SKIP 0x00
#define OVERSAMPLING_x1 0x01
#define OVERSAMPLING_x2 0x02
#define OVERSAMPLING_x4 0x03
#define OVERSAMPLING_x8 0x04
#define OVERSAMPLING_x16 0x05

#define SAPHBME280_SENSOR_MODE_SLEEP 0
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

int32_t saphBme280_init(uint8_t address, saphBmeDevice_t* device);

int32_t saphBme280_getId(saphBmeDevice_t* device);

int32_t saphBme280_resetDevice(saphBmeDevice_t* device);

void saphBme280_prepareMeasureCtrlReg(saphBmeDevice_t* device, uint8_t tempOversampling, uint8_t pressureOversampling,
                                      uint8_t sensorMode);

int32_t saphBme280_commitMeasureCtrlReg(saphBmeDevice_t* device);

void saphBme280_prepareConfigReg(saphBmeDevice_t* device, uint8_t standbyTime, uint8_t iirFilterCoefficient);

int32_t saphBme280_commitConfigReg(saphBmeDevice_t* device);

void saphBme280_prepareCtrlHumidityReg(saphBmeDevice_t* device, uint8_t humidityOversampling);

int32_t saphBme280_commitCtrlHumidity(saphBmeDevice_t* device);

int32_t saphBme280_status(saphBmeDevice_t* device, uint8_t* buffer);

int32_t saphBme280_getMeasurements(saphBmeDevice_t* device, saphBmeMeasurements_t* result);

int32_t saphBme280_getPressure(saphBmeDevice_t* device, uint32_t* resultBuffer);

#endif // SAPHBME280_H
