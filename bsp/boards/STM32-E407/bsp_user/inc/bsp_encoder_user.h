#ifndef BSP_ENCODER_USER_H
#define BSP_ENCODER_USER_H

#include "bsp.h"

typedef enum
{
    BSP_ENCODER_USER_TIMER_MAX
} BspEncoderUser_Timer_t;

extern Bsp_EncoderHandle_t BspEncoderUser_HandleTable[1U];

Bsp_EncoderHandle_t *BspEncoderUser_GetEncoderHandle(const Bsp_TimerHandle_t *const timer_handle);

#endif /* BSP_ENCODER_USER_H */