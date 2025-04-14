#include "bsp_adc_user.h"

#include <stdint.h>

#include "adc.h"

#include "bsp.h"

#define BSP_ADC_USER_ADC_1_CHANNELS 1U

static uint32_t         BspAdcUser_Adc1Buffer[BSP_ADC_USER_ADC_1_CHANNELS];
static Bsp_AdcReading_t BspAdcUser_Adc1ShadowBuffer[BSP_ADC_USER_ADC_1_CHANNELS];

Bsp_Adc_t BspAdcUser_HandleTable[BSP_ADC_USER_ADC_MAX] = {
    [BSP_ADC_USER_ADC_1] = {
        .adc_handle    = &hadc3,
        .buffer        = BspAdcUser_Adc1Buffer,
        .shadow_buffer = BspAdcUser_Adc1ShadowBuffer,
        .channels      = BSP_ADC_USER_ADC_1_CHANNELS,
    },
};

Bsp_Adc_t* BspAdcUser_GetAdc(const Bsp_AdcHandle_t *const adc_handle)
{
    Bsp_Adc_t* adc = NULL;

    if (adc_handle == BspAdcUser_HandleTable[BSP_ADC_USER_ADC_1].adc_handle)
    {
        adc = &BspAdcUser_HandleTable[BSP_ADC_USER_ADC_1];
    }

    return adc;
}