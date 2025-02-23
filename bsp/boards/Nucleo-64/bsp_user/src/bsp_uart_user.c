#include "bsp_uart_user.h"

#include <stdint.h>

#include "usart.h"

#include "bsp.h"

#define BSP_UART_USER_BUFFER_SIZE 512U

static uint8_t BspUartUser_LogTxBuffer[BSP_UART_USER_BUFFER_SIZE];
static uint8_t BspUartUser_LogRxBuffer[BSP_UART_USER_BUFFER_SIZE];
static uint8_t BspUartUser_CommsTxBuffer[BSP_UART_USER_BUFFER_SIZE];
static uint8_t BspUartUser_CommsRxBuffer[BSP_UART_USER_BUFFER_SIZE];

Bsp_Uart_t BspUartUser_HandleTable[BSP_UART_USER_MAX] = {
    [BSP_UART_USER_LOG] = {
        .uart_handle    = &huart2,
        .tx_buffer      = BspUartUser_LogTxBuffer,
        .tx_buffer_size = sizeof(BspUartUser_LogTxBuffer),
        .rx_buffer      = BspUartUser_LogRxBuffer,
        .rx_buffer_size = sizeof(BspUartUser_LogRxBuffer),
    },
    [BSP_UART_USER_COMMS] = {
        .uart_handle    = &huart6,
        .tx_buffer      = BspUartUser_CommsTxBuffer,
        .tx_buffer_size = sizeof(BspUartUser_CommsTxBuffer),
        .rx_buffer      = BspUartUser_CommsRxBuffer,
        .rx_buffer_size = sizeof(BspUartUser_CommsRxBuffer),
    }
};

BspUartUser_Uart_t BspUartUser_GetUart(const Bsp_UartHandle_t *const uart_handle)
{
    BspUartUser_Uart_t uart = BSP_UART_USER_MAX;

    if (uart_handle == BspUartUser_HandleTable[BSP_UART_USER_LOG].uart_handle)
    {
        uart = BSP_UART_USER_LOG;
    }
    else if (uart_handle == BspUartUser_HandleTable[BSP_UART_USER_COMMS].uart_handle)
    {
        uart = BSP_UART_USER_COMMS;
    }

    return uart;
}