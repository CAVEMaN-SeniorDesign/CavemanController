#include "bsp_encoder_user.h"

#include "bsp.h"

Bsp_Encoder_t BspEncoderUser_HandleTable[1U];

Bsp_Encoder_t *BspEncoderUser_GetEncoderHandle(const Bsp_TimerHandle_t *const timer_handle)
{
    BSP_UNUSED(timer_handle);

    return NULL;
}