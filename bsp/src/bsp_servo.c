#include "bsp_servo.h"

#include "bsp_pwm.h"

static double BspServo_Map(const double value, const double in_min, const double in_max, const double out_min, const double out_max);

Bsp_Error_t BspServo_Start(BspServo_Handle_t *const handle)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if (NULL != handle)
    {
        error = BspPwm_Start(handle->timer, handle->channel);
    }

    return error;
}

Bsp_Error_t BspServo_Stop(BspServo_Handle_t *const handle)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if (NULL != handle)
    {
        error = BspPwm_Stop(handle->timer, handle->channel);
    }

    return error;
}

Bsp_Error_t BspServo_SetDutyCycle(BspServo_Handle_t *const handle, const Bsp_Percent_t duty_cycle)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if (NULL == handle)
    {
    }
    else if ((duty_cycle > handle->maximum_duty_cycle) || (duty_cycle < handle->minimum_duty_cycle))
    {
        error = BSP_ERROR_VALUE;
    }
    else
    {
        error = BspPwm_SetDutyCycle(handle->timer, handle->channel, duty_cycle);
    }

    return error;
}

Bsp_Error_t BspServo_SetAngle(BspServo_Handle_t *const handle, const Bsp_Radian_t angle)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if (NULL == handle)
    {
    }
    else if ((angle > handle->maximum_angle) || (angle < handle->minimum_angle))
    {
        error = BSP_ERROR_VALUE;
    }
    else
    {
        error = BspPwm_SetDutyCycle(handle->timer,
                                    handle->channel,
                                    BspServo_Map(angle, handle->minimum_angle, handle->maximum_angle, handle->minimum_duty_cycle, handle->maximum_duty_cycle));
    }

    return error;
}

static double BspServo_Map(const double value, const double in_min, const double in_max, const double out_min, const double out_max)
{
    double capped_value = value;

    if (value < in_min)
    {
        capped_value = in_min;
    }
    if (value > in_max)
    {
        capped_value = in_max;
    }

    return (capped_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}