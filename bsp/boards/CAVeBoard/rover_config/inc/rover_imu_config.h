#ifndef ROVER_IMU_CONFIG_H
#define ROVER_IMU_CONFIG_H

#include <stdbool.h>

bool RoverImuConfig_Initialize(void);
bool RoverImuConfig_ReadGyroscope(double *const x, double *const y, double *const z);

#endif /* ROVER_IMU_CONFIG_H */