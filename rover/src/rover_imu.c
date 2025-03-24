#include "rover_imu.h"

#include "bsp.h"
#include "bsp_gpio.h"
#include "bsp_gpio_user.h"
#include "bsp_logger.h"

#include "rover_imu_config.h"

static const char * kRoverImu_LogTag = "ROVER IMU";

void RoverImu_Initialize(void)
{
    (void)BspGpio_Write(BSP_GPIO_USER_PIN_IMU_STATUS, BSP_GPIO_STATE_RESET);

    if (!RoverImuConfig_Initialize())
    {
        BSP_LOGGER_LOG_ERROR(kRoverImu_LogTag, "Failed to initialize");
    }
    else
    {
        (void)BspGpio_Write(BSP_GPIO_USER_PIN_IMU_STATUS, BSP_GPIO_STATE_SET);
        BSP_LOGGER_LOG_INFO(kRoverImu_LogTag, "Initialized");
    }
}

Rover_Error_t RoverImu_ReadGyroscope(Rover_GyroscopeReading_t *const reading)
{
    Rover_Error_t error = ROVER_ERROR_NONE;

    if (NULL == reading)
    {
        error = ROVER_ERROR_NULL;
    }
    else
    {
        /* TODO return type */
        RoverImuConfig_ReadGyroscope(&reading->x, &reading->y, &reading->z);
    }

    return error;
}