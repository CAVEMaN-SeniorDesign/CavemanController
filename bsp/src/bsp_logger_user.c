#include "bsp_logger_user.h"

#ifdef BSP_LOGGER_USER_CUSTOM_LOGGER
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "bsp_logger.h"
#include "bsp_uart.h"
#include "bsp_uart_user.h"

#define BSP_LOGGER_USER_LOG_BUFFER_SIZE 1024U

typedef enum {
    BSP_LOGGER_USER_BUFFER_ID_0,
    BSP_LOGGER_USER_BUFFER_ID_1,
    BSP_LOGGER_USER_BUFFER_ID_MAX
} BspLoggerUser_BufferId_t;

static char    BspLoggerUser_LogBuffer[BSP_LOGGER_USER_LOG_BUFFER_SIZE];
static uint8_t BspLoggerUser_TransmitBuffer[BSP_LOGGER_USER_BUFFER_ID_MAX][BSP_LOGGER_USER_LOG_BUFFER_SIZE];
static volatile uint32_t BspLoggerUser_TransmitBufferWriteCount[BSP_LOGGER_USER_BUFFER_ID_MAX] = {0U, 0U};
// static volatile bool BspLoggerUser_Locks[BSP_LOGGER_USER_BUFFER_ID_MAX] = {false, false};
static volatile bool BspLoggerUser_CriticalSection = false;
static volatile uint8_t BspLoggerUser_Unlocked = 0U;
static volatile bool BspLoggerUser_Transmitting = false;

static void BspLoggerUser_CustomLogger(const char *const buffer, const size_t size);
#endif /* BSP_LOGGER_USER_CUSTOM_LOGGER */

void BspLoggerUser_RegisterCustomLogger(void)
{
#ifdef BSP_LOGGER_USER_CUSTOM_LOGGER
    BspLogger_RegisterCustomLogger(BspLoggerUser_CustomLogger, BspLoggerUser_LogBuffer, sizeof(BspLoggerUser_LogBuffer));
#endif /* BSP_LOGGER_USER_CUSTOM_LOGGER */
}

void BspLoggerUser_TransmitCallback(Bsp_UartHandle_t* uart_handle)
{
#ifdef BSP_LOGGER_USER_CUSTOM_LOGGER
    BSP_UNUSED(uart_handle);

    BspLoggerUser_TransmitBufferWriteCount[BspLoggerUser_Unlocked ^ 1U] = 0U;

    if (BspLoggerUser_CriticalSection)
    {
        BspLoggerUser_Transmitting = false;
    }
    else if (0 == BspLoggerUser_TransmitBufferWriteCount[BspLoggerUser_Unlocked])
    {
        BspLoggerUser_Transmitting = false;
    }
    else
    {
        uint8_t locked = BspLoggerUser_Unlocked;
        BspLoggerUser_Unlocked ^= 1U;

        BspUart_Transmit(BSP_UART_USER_LOG, BspLoggerUser_TransmitBuffer[locked], BspLoggerUser_TransmitBufferWriteCount[locked]);
    }


#endif /* BSP_LOGGER_USER_CUSTOM_LOGGER */
}

#ifdef BSP_LOGGER_USER_CUSTOM_LOGGER
static void BspLoggerUser_CustomLogger(const char *const buffer, const size_t size)
{
    BspLoggerUser_CriticalSection = true;
    uint8_t unlocked = BspLoggerUser_Unlocked;
    uint32_t size_remaining = (uint32_t)sizeof(BspLoggerUser_TransmitBuffer[unlocked]) - BspLoggerUser_TransmitBufferWriteCount[unlocked];
    uint32_t write_count = size;

    if (size_remaining < size)
    {
        write_count = size_remaining;
    }

    memcpy((void *)((uint32_t)BspLoggerUser_TransmitBuffer[unlocked] + BspLoggerUser_TransmitBufferWriteCount[unlocked]), buffer, write_count);

    BspLoggerUser_TransmitBufferWriteCount[unlocked] += write_count;
    BspLoggerUser_CriticalSection = false;

    if (!BspLoggerUser_Transmitting)
    {
        uint8_t locked = BspLoggerUser_Unlocked;
        BspLoggerUser_Unlocked ^= 1U;
        BspLoggerUser_Transmitting = true;
        BspUart_Transmit(BSP_UART_USER_LOG, BspLoggerUser_TransmitBuffer[locked], BspLoggerUser_TransmitBufferWriteCount[locked]);
    }
    
}
#endif /* BSP_LOGGER_USER_CUSTOM_LOGGER */