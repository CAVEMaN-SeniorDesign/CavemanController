#ifndef ROVER_IMU_H
#define ROVER_IMU_H

#include <stdint.h>

#include "rover.h"

void RoverImu_Initialize(void);
Rover_Error_t RoverImu_ReadGyroscope(Rover_GyroscopeReading_t *const reading);

#endif /* ROVER_IMU_H */