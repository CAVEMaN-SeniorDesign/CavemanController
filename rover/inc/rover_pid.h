#ifndef ROVER_PID_H
#define ROVER_PID_H

#include "rover.h"

typedef struct
{
    double Kp;
    double Ki;
    double Kd;
    double Integral;
    double Error;
    double Output;
} RoverPid_Handle_t;

Rover_Error_t RoverPid_Update(RoverPid_Handle_t *const handle, const double command, const double actual, const Rover_Microsecond_t delta);

#endif // ROVER_PID_H