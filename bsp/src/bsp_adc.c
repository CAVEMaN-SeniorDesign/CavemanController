#include "bsp_adc.h"

#include <string.h>

#include "stm32f4xx_hal.h"

#include "bsp.h"
#include "bsp_adc_user.h"

static void BspAdc_ConversionCompleteCallback(Bsp_AdcHandle_t *adc_handle);

Bsp_Error_t BspAdc_Start(const BspAdcUser_Adc_t adc)
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (adc < BSP_ADC_USER_ADC_MAX)
    {
        BspAdcUser_HandleTable[adc].adc_handle->ConvCpltCallback = BspAdc_ConversionCompleteCallback;

        error = (Bsp_Error_t)HAL_ADC_Start_DMA(BspAdcUser_HandleTable[adc].adc_handle, BspAdcUser_HandleTable[adc].buffer, BspAdcUser_HandleTable[adc].channels);
    }

    return error;
}

Bsp_Error_t BspAdc_Read(const BspAdcUser_Adc_t adc, const BspAdcUser_ChannelRank_t channel_rank, Bsp_AdcReading_t *const adc_reading)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if (NULL == adc_reading)
    {
        error = BSP_ERROR_NULL;
    }
    else if ((adc >= BSP_ADC_USER_ADC_MAX) || (channel_rank >= BspAdcUser_HandleTable[adc].channels))
    {
        error = BSP_ERROR_PERIPHERAL;
    }
    else
    {
        *adc_reading = *(BspAdcUser_HandleTable[adc].shadow_buffer + channel_rank);
    }

    return error;
}

static void BspAdc_ConversionCompleteCallback(Bsp_AdcHandle_t *adc_handle)
{
    Bsp_Adc_t *adc = BspAdcUser_GetAdc(adc_handle);

    if (NULL != adc)
    {
        for (uint8_t i = 0; i < adc->channels; i++)
        {
            *(adc->shadow_buffer + i) = (uint16_t)*(adc->buffer + i);
        }
    }
}