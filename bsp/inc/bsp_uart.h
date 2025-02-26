#ifndef BSP_UART_H
#define BSP_UART_H

#include <stddef.h>

#include "bsp.h"
#include "bsp_uart_user.h"

Bsp_Error_t BspUart_Start(const BspUartUser_Uart_t uart, void (*tx_callback)(Bsp_Uart_t *const uart), void (*rx_callback)(Bsp_Uart_t *const uart));
Bsp_Error_t BspUart_StartTransmit(const BspUartUser_Uart_t uart, const uint8_t *const data, const size_t size);
// Bsp_Error_t BspUart_StartReceive(const BspUartUser_Uart_t uart, const size_t size);
Bsp_Error_t BspUart_Available(const BspUartUser_Uart_t uart, size_t *const bytes_available);
Bsp_Error_t BspUart_Read(const BspUartUser_Uart_t uart, uint8_t *const data, const size_t size, size_t *const bytes_read);

#endif /* BSP_UART_H */