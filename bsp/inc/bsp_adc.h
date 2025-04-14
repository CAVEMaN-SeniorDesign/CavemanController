#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "bsp.h"
#include "bsp_adc_user.h"

Bsp_Error_t BspAdc_Start(const BspAdcUser_Adc_t adc);
Bsp_Error_t BspAdc_Read(const BspAdcUser_Adc_t adc, const BspAdcUser_ChannelRank_t channel_rank, Bsp_AdcReading_t *const adc_reading);

#endif /* BSP_ADC_H */