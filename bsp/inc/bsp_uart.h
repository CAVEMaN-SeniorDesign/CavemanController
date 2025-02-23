#ifndef BSP_UART_H
#define BSP_UART_H

#include <stddef.h>

#include "bsp.h"
#include "bsp_uart_user.h"

Bsp_Error_t BspUart_Start(const BspUartUser_Uart_t uart, void (*transmit_callback)(Bsp_UartHandle_t *handle), void (*receive_callback)(Bsp_UartHandle_t *handle));
Bsp_Error_t BspUart_Transmit(const BspUartUser_Uart_t uart, const uint8_t *const data, const size_t size);
Bsp_Error_t BspUart_Receive(const BspUartUser_Uart_t uart, uint8_t *const data, const size_t size);

#endif /* BSP_UART_H */