#ifndef ROVER_PID_H
#define ROVER_PID_H

#include "rover.h"

typedef struct
{
    double kp;
    double ki;
    double kd;
    double integral;
    double command;
    double error;
    double output;
} RoverPid_Handle_t;

Rover_Error_t RoverPid_Reset(RoverPid_Handle_t *const handle);
Rover_Error_t RoverPid_SetCommand(RoverPid_Handle_t *const handle, const double command);
Rover_Error_t RoverPid_Update(RoverPid_Handle_t *const handle, const double actual, const Rover_Microsecond_t delta);

#endif /* ROVER_PID_H */