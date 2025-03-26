#include "caveman_buttons.h"

#include "bsp.h"
#include "bsp_gpio.h"
#include "bsp_gpio_user.h"
#include "bsp_logger.h"
#include "bsp_tick.h"

typedef struct
{
    BspGpioUser_Pin_t pin;
    Bsp_GpioState_t trigger_state;
    Bsp_Microsecond_t interval_min;
    Bsp_Microsecond_t interval_max;
    Bsp_GpioState_t previous_state;
    Bsp_Microsecond_t previous_tick;
    void (*on_press)(const BspGpioUser_Pin_t pin);
} CavemanButtons_Handle_t;

static const char *kCavemanButtons_LogTag = "BUTTONS";

static void CavemanButtons_OnPress(const Bsp_GpioPin_t pin);
static void CavemanButtons_HeadlightsCallback(const BspGpioUser_Pin_t pin);

static CavemanButtons_Handle_t CavemanButtons_HandleTable[CAVEMAN_BUTTONS_BUTTON_MAX] = {
    [CAVEMAN_BUTTONS_BUTTON_HEADLIGHTS] = {
        .pin            = BSP_GPIO_USER_PIN_HEADLIGHTS_ENABLE,
        .trigger_state  = BSP_GPIO_STATE_SET,
        .interval_min   = 100000U,
        .interval_max   = 200000U,
        .previous_state = BSP_GPIO_STATE_RESET,
        .previous_tick  = 0U,
        .on_press       = CavemanButtons_HeadlightsCallback,
    },
};

Bsp_Error_t CavemanButtons_Enable(const CavemanButtons_Button_t button)
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (button < CAVEMAN_BUTTONS_BUTTON_MAX)
    {
        error = BspGpio_RegisterCallback(CavemanButtons_HandleTable[button].pin, CavemanButtons_OnPress);
    }

    return error;
}

Bsp_Error_t CavemanButtons_Disable(const CavemanButtons_Button_t button)
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (button < CAVEMAN_BUTTONS_BUTTON_MAX)
    {
        error = BspGpio_RegisterCallback(CavemanButtons_HandleTable[button].pin, NULL);
    }

    return error;
}

static void CavemanButtons_OnPress(const Bsp_GpioPin_t pin)
{
    CavemanButtons_Button_t button     = CAVEMAN_BUTTONS_BUTTON_MAX;
    Bsp_GpioState_t         gpio_state = BSP_GPIO_STATE_RESET;
    Bsp_Microsecond_t       tick       = BspTick_GetMicroseconds();

    if (pin == BspGpioUser_HandleTable[CavemanButtons_HandleTable[CAVEMAN_BUTTONS_BUTTON_HEADLIGHTS].pin].gpio_pin)
    {
        button = CAVEMAN_BUTTONS_BUTTON_HEADLIGHTS;
    }
    /* TODO SD-130 add other buttons here */

    if ((button >= CAVEMAN_BUTTONS_BUTTON_MAX) || (BSP_ERROR_NONE != BspGpio_Read(CavemanButtons_HandleTable[button].pin, &gpio_state)))
    {
    }
    else if (gpio_state == CavemanButtons_HandleTable[button].trigger_state)
    {
        CavemanButtons_HandleTable[button].previous_state = gpio_state;
        CavemanButtons_HandleTable[button].previous_tick  = tick;
    }
    else if ((CavemanButtons_HandleTable[button].previous_state == CavemanButtons_HandleTable[button].trigger_state) &&
             ((tick - CavemanButtons_HandleTable[button].previous_tick) >= CavemanButtons_HandleTable[button].interval_min) &&
             ((tick - CavemanButtons_HandleTable[button].previous_tick) <= CavemanButtons_HandleTable[button].interval_max))
    {
        CavemanButtons_HandleTable[button].previous_state = gpio_state;

        if (NULL != CavemanButtons_HandleTable[button].on_press)
        {
            CavemanButtons_HandleTable[button].on_press(pin);
        }
    }
}

static void CavemanButtons_HeadlightsCallback(const BspGpioUser_Pin_t pin)
{
    BSP_UNUSED(pin);

    (void)BspGpio_Toggle(BSP_GPIO_USER_PIN_HEADLIGHTS_0);
    (void)BspGpio_Toggle(BSP_GPIO_USER_PIN_HEADLIGHTS_1);
    (void)BspGpio_Toggle(BSP_GPIO_USER_PIN_HEADLIGHTS_2);

    BSP_LOGGER_LOG_INFO(kCavemanButtons_LogTag, "Toggle headlights");
}