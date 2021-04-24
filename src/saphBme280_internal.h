#ifndef SAPHBME280_INTERNAL_H
#define SAPHBME280_INTERNAL_H

#include "saphBme280.h"

saphBmeMeasurements_t
saphBme280_compensateMeasurements(saphBmeDevice_t* device, saphBmeRawMeasurements_t* rawMeasurements);


#endif // SAPHBME280_INTERNAL_H
