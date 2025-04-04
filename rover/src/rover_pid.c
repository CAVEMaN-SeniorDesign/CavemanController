#include "rover_pid.h"

#include "rover.h"

Rover_Error_t RoverPid_Reset(RoverPid_Handle_t *const handle)
{
    Rover_Error_t error = ROVER_ERROR_NULL;

    if (NULL != handle)
    {
        handle->integral      = 0.0;
        handle->command       = 0.0;
        handle->error         = 0.0;
        handle->output        = 0.0;
        handle->previous_tick = 0U;
    }

    return error;
}

Rover_Error_t RoverPid_Update(RoverPid_Handle_t *const handle, const double actual, const Rover_Microsecond_t tick)
{
    Rover_Error_t error = ROVER_ERROR_NULL;

    if (NULL != handle)
    {
        Rover_Microsecond_t delta      = handle->previous_tick - tick;
        double              pid_error  = handle->command - actual;
        double              derivative = (pid_error - handle->error) / delta;

        handle->integral     += pid_error * delta;
        handle->output        = (handle->kp * pid_error) + (handle->ki * handle->integral) + (handle->kd * derivative);
        handle->error         = pid_error;
        handle->previous_tick = tick;
    }

    return error;
}