#include "rover_imu.h"

#include "bsp_logger.h"

#include "rover_imu_config.h"

static const char * kRoverImu_LogTag = "ROVER IMU";

void RoverImu_Initialize(void)
{
    if (!RoverImuConfig_Initialize())
    {
        BSP_LOGGER_LOG_ERROR(kRoverImu_LogTag, "Failed to initialize");
    }
    else
    {
        BSP_LOGGER_LOG_INFO(kRoverImu_LogTag, "Initialized");
    }
}