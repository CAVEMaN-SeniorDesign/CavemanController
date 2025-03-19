#include "caveman_controller.h"

#include <stdbool.h>

#include "bsp.h"
#include "bsp_gpio.h"
#include "bsp_gpio_user.h"
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

static const char *kCaveman_LogTag = "CAVEMAN";

static void Caveman_Initialize(void);
static void Caveman_HeadlightsCallback(const Bsp_GpioPin_t pin);

int main(void)
{
    Caveman_Initialize();
    Rover_SetMode(ROVER_MODE_RUN);

    size_t            loop_count = 0U;
    Bsp_Microsecond_t start      = BspTick_GetMicroseconds();
    while (true)
    {
        Rover_Task();
        CavemanCaveTalk_Task();
        loop_count++;

        Bsp_Microsecond_t time       = BspTick_GetMicroseconds();
        Bsp_Microsecond_t difference = time - start;
        if (difference >= 1e6)
        {
            int loop_rate = (int)((double)loop_count / ((double)difference / 10e6));
            BSP_LOGGER_LOG_INFO(kCaveman_LogTag, "Loop rate %dHz", loop_rate);
            loop_count = 0;
            start      = time;
        }
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
        BSP_LOGGER_LOG_ERROR(kCaveman_LogTag, "Failed to start CAVeTalk");
    }
    BSP_LOGGER_LOG_DEBUG(kCaveman_LogTag, "Initialized");

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0, GPIO_PIN_SET); /* Test IMU LED */

    /* Turn off headlights */
    (void)BspGpio_Write(BSP_GPIO_USER_PIN_HEADLIGHTS_0, BSP_GPIO_STATE_RESET);
    (void)BspGpio_Write(BSP_GPIO_USER_PIN_HEADLIGHTS_1, BSP_GPIO_STATE_RESET);
    (void)BspGpio_Write(BSP_GPIO_USER_PIN_HEADLIGHTS_2, BSP_GPIO_STATE_RESET);

    (void)BspGpio_RegisterCallback(BSP_GPIO_USER_PIN_HEADLIGHTS_ENABLE, Caveman_HeadlightsCallback);
}

static void Caveman_HeadlightsCallback(const Bsp_GpioPin_t pin)
{
    BSP_UNUSED(pin);

    (void)BspGpio_Toggle(BSP_GPIO_USER_PIN_HEADLIGHTS_0);
    (void)BspGpio_Toggle(BSP_GPIO_USER_PIN_HEADLIGHTS_1);
    (void)BspGpio_Toggle(BSP_GPIO_USER_PIN_HEADLIGHTS_2);

    BSP_LOGGER_LOG_INFO(kCaveman_LogTag, "Toggle headlights");
}
