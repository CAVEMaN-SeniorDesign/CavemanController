#ifndef BSP_PWM_USER_H
#define BSP_PWM_USER_H

#include "bsp.h"

typedef enum
{
    BSP_PWM_USER_TIMER_MOTORS_FRONT,
    BSP_PWM_USER_TIMER_MOTORS_REAR,
    BSP_PWM_USER_TIMER_SERVOS_FRONT,
    BSP_PWM_USER_TIMER_SERVOS_REAR,
    BSP_PWM_USER_TIMER_CAMERA_PAN,
    BSP_PWM_USER_TIMER_CAMERA_TILT,
    BSP_PWM_USER_TIMER_MAX
} BspPwmUser_Timer_t;

extern const Bsp_PwmConfig_t BspPwmUser_TimerConfigTable[BSP_PWM_USER_TIMER_MAX];

#endif /* BSP_PWM_USER_H */