#include "rover_imu_config.h"

#include <stdbool.h>
#include <stdint.h>

#include "lsm6dsv16x_reg.h"
#include "spi.h"
#include "stm32f4xx_hal.h"

#include "bsp.h"
#include "bsp_gpio.h"
#include "bsp_gpio_user.h"
#include "bsp_logger.h"

#define ROVER_IMU_BOOT_TIME     (Bsp_Millisecond_t)10U
#define ROVER_IMU_TIMEOUT       (Bsp_Millisecond_t)1000U
#define ROVER_IMU_REGISTER_READ 0x80U

static const char * kRoverImuConfig_LogTag = "ROVER IMU CONFIG";

static int32_t RoverImuConfig_Write(void *const handle, const uint8_t imu_register, const uint8_t *const data, const uint16_t size);
static int32_t RoverImuConfig_Read(void *const handle, const uint8_t imu_register, uint8_t *const data, const uint16_t size);

static stmdev_ctx_t RoverImuConfig_DeviceHandle = {
    .write_reg = RoverImuConfig_Write,
    .read_reg  = RoverImuConfig_Read,
    .mdelay    = Bsp_Delay,
    .handle    = &hspi2,
};

static int16_t RoverImuConfig_RawGyroscope[3U]; /* TODO magic number */

bool RoverImuConfig_Initialize(void)
{
    bool initialized = false;

    /* Wait sensor boot time */
    Bsp_Delay(ROVER_IMU_BOOT_TIME);

    /* Check device ID */
    uint8_t whoami;
    lsm6dsv16x_device_id_get(&RoverImuConfig_DeviceHandle, &whoami);
    if (LSM6DSV16X_ID != whoami)
    {
        BSP_LOGGER_LOG_ERROR(kRoverImuConfig_LogTag, "Failed to detect IMU");
    }
    else
    {
        BSP_LOGGER_LOG_DEBUG(kRoverImuConfig_LogTag, "IMU detected");

        /* Restore default configuration */
        BSP_LOGGER_LOG_DEBUG(kRoverImuConfig_LogTag, "Resetting IMU");
        lsm6dsv16x_reset_t imu_reset;
        lsm6dsv16x_reset_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_RESTORE_CTRL_REGS);
        lsm6dsv16x_reset_get(&RoverImuConfig_DeviceHandle, &imu_reset);
        while (LSM6DSV16X_READY != imu_reset)
        {
            lsm6dsv16x_reset_get(&RoverImuConfig_DeviceHandle, &imu_reset);
        }
        BSP_LOGGER_LOG_DEBUG(kRoverImuConfig_LogTag, "IMU reset");

        /* Enable Block Data Update */
        lsm6dsv16x_block_data_update_set(&RoverImuConfig_DeviceHandle, PROPERTY_ENABLE);

        /* Set Output Data Rate.
         * Selected data rate have to be equal or greater with respect
         * with MLC data rate.
         */
        lsm6dsv16x_xl_data_rate_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_ODR_AT_7680Hz);
        lsm6dsv16x_gy_data_rate_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_ODR_AT_7680Hz);

        /* Set full scale */
        lsm6dsv16x_xl_full_scale_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_2g);
        lsm6dsv16x_gy_full_scale_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_2000dps);

        /* Configure filtering chain */
        lsm6dsv16x_filt_settling_mask_t filter_settling_mask;
        filter_settling_mask.drdy   = PROPERTY_ENABLE;
        filter_settling_mask.irq_xl = PROPERTY_ENABLE;
        filter_settling_mask.irq_g  = PROPERTY_ENABLE;
        lsm6dsv16x_filt_settling_mask_set(&RoverImuConfig_DeviceHandle, filter_settling_mask);
        lsm6dsv16x_filt_gy_lp1_set(&RoverImuConfig_DeviceHandle, PROPERTY_ENABLE);
        lsm6dsv16x_filt_gy_lp1_bandwidth_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_GY_ULTRA_LIGHT);
        lsm6dsv16x_filt_xl_lp2_set(&RoverImuConfig_DeviceHandle, PROPERTY_ENABLE);
        lsm6dsv16x_filt_xl_lp2_bandwidth_set(&RoverImuConfig_DeviceHandle, LSM6DSV16X_XL_STRONG);

        initialized = true;

        BSP_LOGGER_LOG_DEBUG(kRoverImuConfig_LogTag, "Initialized");
    }

    return initialized;
}

bool RoverImuConfig_ReadGyroscope(double *const x, double *const y, double *const z)
{
    bool read = false;

    if ((NULL == x) || (NULL == y) || (NULL == z))
    {
    }
    else
    {
        lsm6dsv16x_data_ready_t data_ready;
        lsm6dsv16x_flag_data_ready_get(&RoverImuConfig_DeviceHandle, &data_ready);

        if (data_ready.drdy_gy)
        {
            lsm6dsv16x_angular_rate_raw_get(&RoverImuConfig_DeviceHandle, RoverImuConfig_RawGyroscope);
        }

        /* TODO convert to rad/s */
        *x = (double)lsm6dsv16x_from_fs2000_to_mdps(RoverImuConfig_RawGyroscope[0U]);
        *y = (double)lsm6dsv16x_from_fs2000_to_mdps(RoverImuConfig_RawGyroscope[0U]);
        *z = (double)lsm6dsv16x_from_fs2000_to_mdps(RoverImuConfig_RawGyroscope[0U]);

        read = true;
    }

    return read;
}

static int32_t RoverImuConfig_Write(void *const handle, const uint8_t imu_register, const uint8_t *const data, const uint16_t size)
{
    BspGpio_Write(BSP_GPIO_USER_PIN_IMU_CS, BSP_GPIO_STATE_RESET);

    HAL_SPI_Transmit(handle, &imu_register, 1U, ROVER_IMU_TIMEOUT);
    HAL_SPI_Transmit(handle, data, size, ROVER_IMU_TIMEOUT);

    BspGpio_Write(BSP_GPIO_USER_PIN_IMU_CS, BSP_GPIO_STATE_SET);

    return 0;
}

static int32_t RoverImuConfig_Read(void *const handle, const uint8_t imu_register, uint8_t *const data, const uint16_t size)
{
    uint8_t register_read = imu_register | ROVER_IMU_REGISTER_READ;

    BspGpio_Write(BSP_GPIO_USER_PIN_IMU_CS, BSP_GPIO_STATE_RESET);

    HAL_SPI_Transmit(handle, &register_read, 1U, ROVER_IMU_TIMEOUT);
    HAL_SPI_Receive(handle, data, size, ROVER_IMU_TIMEOUT);

    BspGpio_Write(BSP_GPIO_USER_PIN_IMU_CS, BSP_GPIO_STATE_SET);

    return 0;
}