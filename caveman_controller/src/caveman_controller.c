#include "caveman_controller.h"

#include <stdbool.h>

#include "bsp.h"
#include "bsp_encoder.h"
#include "bsp_encoder_user.h"
#include "bsp_logger.h"
#include "bsp_motor.h"
#include "bsp_pwm.h"
#include "bsp_pwm_user.h"
#include "bsp_servo.h"
#include "bsp_tick.h"
#include "bsp_uart.h"
#include "bsp_uart_user.h"

#include "caveman_cavetalk.h"
#include "rover.h"

#include "spi.h"
#include "lsm6dsv16x_reg.h"

#define BOOT_TIME 10

static uint8_t  whoamI = 0U;
static int16_t  data_raw_acceleration[3];
static int16_t  data_raw_angular_rate[3];
static int16_t  data_raw_temperature;
static double_t acceleration_mg[3];
static double_t angular_rate_mdps[3];
static double_t temperature_degC;

static const char *kCaveman_LogTag = "CAVEMAN";

static void Caveman_Initialize(void);

void lsm6dsv16x_read_data_polling(void);
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);
static void platform_init(void);
static lsm6dsv16x_filt_settling_mask_t filt_settling_mask;

int main(void)
{
    Caveman_Initialize();

    /* Example from https://github.com/STMicroelectronics/STMems_Standard_C_drivers/blob/master/lsm6dsv16x_STdC/examples/lsm6dsv16x_read_data_polling.c */
    lsm6dsv16x_read_data_polling();

    Rover_SetMode(ROVER_MODE_RUN);

    while (true)
    {
        Rover_Task();
        CavemanCaveTalk_Task();
    }

    return 0;
}

static void Caveman_Initialize(void)
{
    Bsp_Initialize();
    if (BSP_ERROR_NONE != BspTick_Start())
    {
        BSP_LOGGER_LOG_ERROR(kCaveman_LogTag, "Failed to start BSP Tick");
    }
    Rover_Initialize();
    if (CAVE_TALK_ERROR_NONE != CavemanCaveTalk_Start())
    {
        BSP_LOGGER_LOG_ERROR(kCaveman_LogTag, "Failed to start Rover");
    }
    BSP_LOGGER_LOG_DEBUG(kCaveman_LogTag, "Initialized");
}

void lsm6dsv16x_read_data_polling(void)
{
    lsm6dsv16x_reset_t rst;
    stmdev_ctx_t       dev_ctx;
    /* Initialize mems driver interface */
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg  = platform_read;
    dev_ctx.mdelay    = platform_delay;
    dev_ctx.handle    = &hspi1;

    /* Init test platform */
    platform_init();
    /* Wait sensor boot time */
    platform_delay(BOOT_TIME);

    /* Check device ID */
    lsm6dsv16x_device_id_get(&dev_ctx, &whoamI);

    if (whoamI != LSM6DSV16X_ID)
        while (1)
            ;

    /* Restore default configuration */
    lsm6dsv16x_reset_set(&dev_ctx, LSM6DSV16X_RESTORE_CTRL_REGS);
    do
    {
        lsm6dsv16x_reset_get(&dev_ctx, &rst);
    }
    while (rst != LSM6DSV16X_READY);

    /* Enable Block Data Update */
    lsm6dsv16x_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate.
     * Selected data rate have to be equal or greater with respect
     * with MLC data rate.
     */
    lsm6dsv16x_xl_data_rate_set(&dev_ctx, LSM6DSV16X_ODR_AT_7Hz5);
    lsm6dsv16x_gy_data_rate_set(&dev_ctx, LSM6DSV16X_ODR_AT_15Hz);
    /* Set full scale */
    lsm6dsv16x_xl_full_scale_set(&dev_ctx, LSM6DSV16X_2g);
    lsm6dsv16x_gy_full_scale_set(&dev_ctx, LSM6DSV16X_2000dps);
    /* Configure filtering chain */
    filt_settling_mask.drdy   = PROPERTY_ENABLE;
    filt_settling_mask.irq_xl = PROPERTY_ENABLE;
    filt_settling_mask.irq_g  = PROPERTY_ENABLE;
    lsm6dsv16x_filt_settling_mask_set(&dev_ctx, filt_settling_mask);
    lsm6dsv16x_filt_gy_lp1_set(&dev_ctx, PROPERTY_ENABLE);
    lsm6dsv16x_filt_gy_lp1_bandwidth_set(&dev_ctx, LSM6DSV16X_GY_ULTRA_LIGHT);
    lsm6dsv16x_filt_xl_lp2_set(&dev_ctx, PROPERTY_ENABLE);
    lsm6dsv16x_filt_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSV16X_XL_STRONG);

    /* Read samples in polling mode (no int) */
    while (1)
    {
        lsm6dsv16x_data_ready_t drdy;

        /* Read output only if new xl value is available */
        lsm6dsv16x_flag_data_ready_get(&dev_ctx, &drdy);

        if (drdy.drdy_xl)
        {
            /* Read acceleration field data */
            memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
            lsm6dsv16x_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
            acceleration_mg[0] =
                lsm6dsv16x_from_fs2_to_mg(data_raw_acceleration[0]);
            acceleration_mg[1] =
                lsm6dsv16x_from_fs2_to_mg(data_raw_acceleration[1]);
            acceleration_mg[2] =
                lsm6dsv16x_from_fs2_to_mg(data_raw_acceleration[2]);
            BSP_LOGGER_LOG_INFO(kCaveman_LogTag, "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n", acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
        }

        /* Read output only if new xl value is available */
        if (drdy.drdy_gy)
        {
            /* Read angular rate field data */
            memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
            lsm6dsv16x_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
            angular_rate_mdps[0] =
                lsm6dsv16x_from_fs2000_to_mdps(data_raw_angular_rate[0]);
            angular_rate_mdps[1] =
                lsm6dsv16x_from_fs2000_to_mdps(data_raw_angular_rate[1]);
            angular_rate_mdps[2] =
                lsm6dsv16x_from_fs2000_to_mdps(data_raw_angular_rate[2]);
            // BSP_LOGGER_LOG_INFO(kCaveman_LogTag, "Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n", angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
        }

        if (drdy.drdy_temp)
        {
            /* Read temperature data */
            memset(&data_raw_temperature, 0x00, sizeof(int16_t));
            lsm6dsv16x_temperature_raw_get(&dev_ctx, &data_raw_temperature);
            temperature_degC = lsm6dsv16x_from_lsb_to_celsius(
                data_raw_temperature);
            // BSP_LOGGER_LOG_INFO(kCaveman_LogTag,
            //                      "Temperature [degC]:%6.2f\r\n", temperature_degC);
        }
    }
}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
/* TODO replace with SPI */
// #if defined(NUCLEO_F401RE)
//   HAL_I2C_Mem_Write(handle, LSM6DSV16X_I2C_ADD_L, reg,
//                     I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);
// #elif defined(STEVAL_MKI109V3)
//   HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
//   HAL_SPI_Transmit(handle, &reg, 1, 1000);
//   HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 1000);
//   HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
// #elif defined(SPC584B_DIS)
//   i2c_lld_write(handle,  LSM6DSV16X_I2C_ADD_H & 0xFE, reg, (uint8_t*) bufp, len);
// #endif
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Transmit(handle, (uint8_t *)bufp, len, 1000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
/* TODO replace with SPI */
// #if defined(NUCLEO_F401RE)
//   HAL_I2C_Mem_Read(handle, LSM6DSV16X_I2C_ADD_L, reg,
//                    I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
// #elif defined(STEVAL_MKI109V3)
//   reg |= 0x80;
//   HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_RESET);
//   HAL_SPI_Transmit(handle, &reg, 1, 1000);
//   HAL_SPI_Receive(handle, bufp, len, 1000);
//   HAL_GPIO_WritePin(CS_up_GPIO_Port, CS_up_Pin, GPIO_PIN_SET);
// #elif defined(SPC584B_DIS)
//   i2c_lld_read(handle, LSM6DSV16X_I2C_ADD_H & 0xFE, reg, bufp, len);
// #endif
    reg |= 0x80;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Receive(handle, bufp, len, 1000);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    return 0;
}

/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void platform_delay(uint32_t ms)
{
    HAL_Delay(ms);
}

/*
 * @brief  platform specific initialization (platform dependent)
 */
static void platform_init(void)
{
    /* No initialized needed for STM32F411RE */
}
