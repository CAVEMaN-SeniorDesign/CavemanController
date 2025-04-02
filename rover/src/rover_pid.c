#include "rover_pid.h"

#include "rover.h"

Rover_Error_t RoverPid_Update(RoverPid_Handle_t *const handle, const double command, const double actual, const Rover_Microsecond_t delta)
{
    Rover_Error_t error = ROVER_ERROR_NULL;

    if (NULL != handle)
    {
        double pid_error = command - actual;
        double derivative = (pid_error - handle->Error) / delta;
        handle->Integral += pid_error * delta;

        handle->Output = (handle->Kp * pid_error) + (handle->Ki * handle->Integral) + (handle->Kd * derivative);
        handle->Error = pid_error;
    }

    return error;
}