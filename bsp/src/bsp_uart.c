#include "bsp_uart.h"

#include "stm32f4xx_hal.h"

#include "bsp.h"
#include "bsp_uart_user.h"

extern void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle);

Bsp_Error_t BspUart_Start(const BspUartUser_Uart_t uart, void (*transmit_callback)(Bsp_UartHandle_t *handle), void (*receive_callback)(Bsp_UartHandle_t *handle))
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (uart < BSP_UART_USER_MAX)
    {
        if (NULL != transmit_callback)
        {
            BspUartUser_HandleTable[uart]->TxCpltCallback = transmit_callback;
        }

        if (NULL != receive_callback)
        {
            BspUartUser_HandleTable[uart]->RxCpltCallback = receive_callback;
        }

        /* TODO SD-234 error and abort callbacks */

        HAL_UART_MspInit(BspUartUser_HandleTable[uart]);
    }

    return error;
}

Bsp_Error_t BspUart_Transmit(const BspUartUser_Uart_t uart, const uint8_t *const data, const size_t size)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if (NULL == data)
    {
    }
    else if (uart >= BSP_UART_USER_MAX)
    {
        error = BSP_ERROR_PERIPHERAL;
    }
    else
    {
        error = (Bsp_Error_t)HAL_UART_Transmit_DMA(BspUartUser_HandleTable[uart], data, size);
    }

    return error;
}

Bsp_Error_t BspUart_Receive(const BspUartUser_Uart_t uart, uint8_t *const data, const size_t size)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if (NULL == data)
    {
    }
    else if (uart >= BSP_UART_USER_MAX)
    {
        error = BSP_ERROR_PERIPHERAL;
    }
    else
    {
        error = (Bsp_Error_t)HAL_UART_Receive_DMA(BspUartUser_HandleTable[uart], data, size);
    }

    return error;
}