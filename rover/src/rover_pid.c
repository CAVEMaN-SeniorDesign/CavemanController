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

        error = ROVER_ERROR_NONE;
    }

    return error;
}

Rover_Error_t RoverPid_Enable(RoverPid_Handle_t *const handle)
{
    Rover_Error_t error = ROVER_ERROR_NULL;

    if (NULL != handle)
    {
        handle->enabled  = true;
        handle->integral = 0.0;

        error = ROVER_ERROR_NONE;
    }

    return error;
}

Rover_Error_t RoverPid_Disable(RoverPid_Handle_t *const handle)
{
    Rover_Error_t error = ROVER_ERROR_NULL;

    if (NULL != handle)
    {
        handle->enabled = false;

        error = ROVER_ERROR_NONE;
    }

    return error;
}

Rover_Error_t RoverPid_Update(RoverPid_Handle_t *const handle, const double actual, const Rover_Microsecond_t tick)
{
    Rover_Error_t error = ROVER_ERROR_NULL;

    if (NULL != handle)
    {
        Rover_Microsecond_t delta      = tick - handle->previous_tick; /* TODO convert microseconds to seconds */
        double              pid_error  = handle->command;
        double              derivative = (pid_error - handle->error) / delta;

        if (handle->enabled)
        {
            pid_error -= actual;
        }

        handle->integral     += pid_error * delta;
        handle->output        = (handle->kp * pid_error) + (handle->ki * handle->integral) + (handle->kd * derivative);
        handle->error         = pid_error;
        handle->previous_tick = tick;

        error = ROVER_ERROR_NONE;
    }

    return error;
}