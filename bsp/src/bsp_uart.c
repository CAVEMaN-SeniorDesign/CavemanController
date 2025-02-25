#include "bsp_uart.h"

#include <stdint.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "bsp.h"
#include "bsp_uart_user.h"

extern void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle);

static void BspUart_TxCallback(Bsp_UartHandle_t *uart_handle);
static void BspUart_RxCallback(Bsp_UartHandle_t *uart_handle);
static Bsp_Error_t BspUart_RxBufferAvailable(Bsp_Uart_t *const uart, uint32_t *const bytes_available);
static Bsp_Error_t BspUart_RxBufferRead(Bsp_Uart_t *const uart, uint8_t *const data, const uint32_t size, uint32_t *bytes_read);
static Bsp_Error_t BspUart_RxBufferStartWrite(Bsp_Uart_t *const uart, const uint32_t size);
static Bsp_Error_t BspUart_RxBufferWriteComplete(Bsp_Uart_t *const uart);
static Bsp_Error_t BspUart_TxBufferWrite(Bsp_Uart_t *const uart, const uint8_t *const data, const uint32_t size);
static Bsp_Error_t BspUart_TxBufferStartRead(Bsp_Uart_t *const uart);
static Bsp_Error_t BspUart_TxBufferReadComplete(Bsp_Uart_t *const uart);
static Bsp_Error_t BspUart_ResetBuffer(Bsp_UartDoubleBuffer_t *const double_buffer);
static inline uint8_t BspUart_ToggleLockedBuffer(Bsp_UartDoubleBuffer_t *const double_buffer);

Bsp_Error_t BspUart_Start(const BspUartUser_Uart_t uart, void (*tx_callback)(Bsp_Uart_t *const uart), void (*rx_callback)(Bsp_Uart_t *const uart))
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (uart < BSP_UART_USER_MAX)
    {
        (void)BspUart_ResetBuffer(&BspUartUser_HandleTable[uart].tx_buffer);
        (void)BspUart_ResetBuffer(&BspUartUser_HandleTable[uart].rx_buffer);

        if (NULL != tx_callback)
        {
            BspUartUser_HandleTable[uart].tx_callback = tx_callback;
        }

        if (NULL != rx_callback)
        {
            BspUartUser_HandleTable[uart].rx_callback = rx_callback;
        }

        /* TODO SD-234 error and abort callbacks */
        BspUartUser_HandleTable[uart].uart_handle->TxCpltCallback = BspUart_TxCallback;
        BspUartUser_HandleTable[uart].uart_handle->RxCpltCallback = BspUart_RxCallback;

        HAL_UART_MspInit(BspUartUser_HandleTable[uart].uart_handle);
    }

    return error;
}

Bsp_Error_t BspUart_StartTransmit(const BspUartUser_Uart_t uart, const uint8_t *const data, const size_t size)
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
        error = BspUart_TxBufferWrite(&BspUartUser_HandleTable[uart], data, size);
    }

    return error;
}

Bsp_Error_t BspUart_StartReceive(const BspUartUser_Uart_t uart, const size_t size)
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (uart < BSP_UART_USER_MAX)
    {
        error = BspUart_RxBufferStartWrite(&BspUartUser_HandleTable[uart], size);
    }

    return error;
}

Bsp_Error_t BspUart_Available(const BspUartUser_Uart_t uart, size_t *const bytes_available)
{
    Bsp_Error_t error = BSP_ERROR_PERIPHERAL;

    if (uart < BSP_UART_USER_MAX)
    {
        uint32_t bytes = 0U;

        error = BspUart_RxBufferAvailable(&BspUartUser_HandleTable[uart], &bytes);

        *bytes_available = (size_t)bytes;
    }

    return error;
}

Bsp_Error_t BspUart_Read(const BspUartUser_Uart_t uart, uint8_t *const data, const size_t size, size_t *const bytes_read)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if ((NULL == data) || (NULL == bytes_read))
    {
    }
    else if (uart >= BSP_UART_USER_MAX)
    {
        error = BSP_ERROR_PERIPHERAL;
    }
    else
    {
        uint32_t bytes = 0U;

        error = BspUart_RxBufferRead(&BspUartUser_HandleTable[uart], data, size, &bytes);

        *bytes_read = (size_t)bytes;
    }

    return error;
}

static void BspUart_TxCallback(Bsp_UartHandle_t *uart_handle)
{
    Bsp_Uart_t* uart = BspUartUser_GetUart(uart_handle);

    if (NULL != uart)
    {
        (void)BspUart_TxBufferReadComplete(uart);

        if (NULL != uart->tx_callback)
        {
            uart->tx_callback(uart);
        }

    }
}

static void BspUart_RxCallback(Bsp_UartHandle_t *uart_handle)
{
    Bsp_Uart_t* uart = BspUartUser_GetUart(uart_handle);

    if (NULL != uart)
    {
        (void)BspUart_RxBufferWriteComplete(uart);

        if (NULL != uart->rx_callback)
        {
            uart->rx_callback(uart);
        }
    }
}

static Bsp_Error_t BspUart_RxBufferAvailable(Bsp_Uart_t *const uart, uint32_t *const bytes_available)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if ((NULL == uart) || (NULL == bytes_available))
    {
        error = BSP_ERROR_NULL;
    }
    else
    {
        Bsp_UartDoubleBuffer_t *double_buffer = &uart->rx_buffer;

        double_buffer->reading = true;

        uint8_t locked = double_buffer->unlocked ^ 1U;
        *bytes_available = double_buffer->write_count[locked] - double_buffer->read_count[locked];

        if ((0U == *bytes_available) && (!double_buffer->writing))
        {
            double_buffer->write_count[locked] = 0U;
            double_buffer->read_count[locked]  = 0U;

            locked = BspUart_ToggleLockedBuffer(double_buffer);

            *bytes_available = double_buffer->write_count[locked] - double_buffer->read_count[locked];
        }

        double_buffer->reading = false;
    }

    return error;
}

static Bsp_Error_t BspUart_RxBufferRead(Bsp_Uart_t *const uart, uint8_t *const data, const uint32_t size, uint32_t *bytes_read)
{
    Bsp_Error_t error = BSP_ERROR_NULL;

    if ((NULL == uart) ||
        (NULL == uart->rx_buffer.buffer) ||
        (NULL == data) ||
        (NULL == bytes_read))
    {
    }
    else
    {
        /* Call available first to toggle locked buffer if necessary */
        error = BspUart_RxBufferAvailable(uart, bytes_read);

        if (BSP_ERROR_NONE == error)
        {
            Bsp_UartDoubleBuffer_t *double_buffer = &uart->rx_buffer;

            double_buffer->reading = true;

            uint8_t              locked      = double_buffer->unlocked ^ 1U;
            uint32_t             offset      = (uint32_t)((uint32_t)locked * double_buffer->half_buffer_size);
            const uint8_t *const buffer_read = (uint8_t *)((uint32_t)double_buffer->buffer + offset + double_buffer->read_count[locked]);

            if (*bytes_read > 0U)
            {
                if (*bytes_read > size)
                {
                    *bytes_read = size;
                }

                memcpy(data, buffer_read, *bytes_read);
                double_buffer->read_count[locked] += *bytes_read;
            }

            double_buffer->reading = false;
        }
    }

    return error;
}

static Bsp_Error_t BspUart_RxBufferStartWrite(Bsp_Uart_t *const uart, const uint32_t size)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if ((NULL == uart) || (NULL == uart->uart_handle) || (NULL == uart->rx_buffer.buffer))
    {
        error = BSP_ERROR_NULL;
    }
    else
    {
        Bsp_UartDoubleBuffer_t *double_buffer = &uart->rx_buffer;

        /* Critical section start */
        double_buffer->writing = true;

        uint8_t  unlocked       = double_buffer->unlocked;
        uint32_t offset         = (uint32_t)((uint32_t)unlocked * double_buffer->half_buffer_size);
        uint32_t size_remaining = double_buffer->half_buffer_size - double_buffer->write_count[unlocked];
        uint8_t *buffer_write   = (uint8_t *)((uint32_t)double_buffer->buffer + offset + double_buffer->write_count[unlocked]);
        uint32_t bytes_to_write = size;

        /* TODO handle truncated bytes */
        if (size_remaining < bytes_to_write)
        {
            bytes_to_write = size_remaining;
        }

        double_buffer->write_count[unlocked] += bytes_to_write;

        if (bytes_to_write > 0U)
        {
            error = (Bsp_Error_t)HAL_UART_Receive_DMA(uart->uart_handle, buffer_write, double_buffer->write_count[unlocked]);
        }
    }

    return error;
}

static Bsp_Error_t BspUart_RxBufferWriteComplete(Bsp_Uart_t *const uart)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if (NULL == uart)
    {
        error = BSP_ERROR_NULL;
    }
    else
    {
        /* Critical section end */
        uart->rx_buffer.writing = false;
    }

    return error;
}

static Bsp_Error_t BspUart_TxBufferWrite(Bsp_Uart_t *const uart, const uint8_t *const data, const uint32_t size)
{
    Bsp_Error_t error = BSP_ERROR_NULL;


    if ((NULL == uart) || (NULL == uart->tx_buffer.buffer) || (NULL == data))
    {
    }
    else
    {
        Bsp_UartDoubleBuffer_t *double_buffer = &uart->tx_buffer;

        /* Critical section start */
        double_buffer->writing = true;

        uint8_t  unlocked       = double_buffer->unlocked;
        uint32_t offset         = (uint32_t)((uint32_t)unlocked * double_buffer->half_buffer_size);
        uint32_t size_remaining = double_buffer->half_buffer_size - double_buffer->write_count[unlocked];
        uint8_t *buffer_write   = (uint8_t *)((uint32_t)double_buffer->buffer + offset + double_buffer->write_count[unlocked]);
        uint32_t bytes_to_write = size;

        /* TODO handle truncated bytes */
        if (size_remaining < bytes_to_write)
        {
            bytes_to_write = size_remaining;
        }

        (void)memcpy(buffer_write, data, bytes_to_write);
        double_buffer->write_count[unlocked] += bytes_to_write;

        /* Critical section end */
        double_buffer->writing = false;

        error = BspUart_TxBufferStartRead(uart);
    }

    return error;
}

static Bsp_Error_t BspUart_TxBufferStartRead(Bsp_Uart_t *const uart)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if ((NULL == uart) || (NULL == uart->uart_handle) || (NULL == uart->tx_buffer.buffer))
    {
        error = BSP_ERROR_NULL;
    }
    else if ((!uart->tx_buffer.reading) && (uart->tx_buffer.write_count[uart->tx_buffer.unlocked] > 0U))
    {
        Bsp_UartDoubleBuffer_t *double_buffer = &uart->tx_buffer;

        uint8_t locked = BspUart_ToggleLockedBuffer(double_buffer);

        double_buffer->reading            = true;
        double_buffer->read_count[locked] = 0U;

        error = (Bsp_Error_t)HAL_UART_Transmit_DMA(uart->uart_handle,
                                                   (uint8_t *)((uint32_t)double_buffer->buffer + (uint32_t)((uint32_t)locked * double_buffer->half_buffer_size)),
                                                   double_buffer->write_count[locked]);
    }

    return error;
}

static Bsp_Error_t BspUart_TxBufferReadComplete(Bsp_Uart_t *const uart)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if (NULL == uart)
    {
        error = BSP_ERROR_NULL;
    }
    else
    {
        Bsp_UartDoubleBuffer_t *double_buffer = &uart->tx_buffer;

        uint8_t locked = double_buffer->unlocked ^ 1U;

        double_buffer->read_count[locked]  = double_buffer->write_count[locked];
        double_buffer->write_count[locked] = 0U;
        double_buffer->reading             = false;

        if (!double_buffer->writing)
        {
            error = BspUart_TxBufferStartRead(uart);
        }
    }

    return error;
}

static Bsp_Error_t BspUart_ResetBuffer(Bsp_UartDoubleBuffer_t *const double_buffer)
{
    Bsp_Error_t error = BSP_ERROR_NONE;

    if (NULL == double_buffer)
    {
        error = BSP_ERROR_NULL;
    }
    else
    {
        double_buffer->writing  = false;
        double_buffer->reading  = false;
        double_buffer->unlocked = 0U;

        for (size_t i = 0U; i < sizeof(double_buffer->write_count); i++)
        {
            double_buffer->write_count[i] = 0U;
            double_buffer->read_count[i]  = 0U;
        }
    }

    return error;
}

static inline uint8_t BspUart_ToggleLockedBuffer(Bsp_UartDoubleBuffer_t *const double_buffer)
{
    uint8_t locked = double_buffer->unlocked;
    double_buffer->unlocked ^= 1U;

    return locked;
}