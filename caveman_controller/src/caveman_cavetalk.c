#include "caveman_cavetalk.h"

#include <stddef.h>

#include "cave_talk.h"
#include "cave_talk_types.h"
#include "cave_talk_link.h"

#include "bsp.h"
#include "bsp_uart.h"
#include "bsp_uart_user.h"

#define CAVEMAN_CAVE_TALK_BUFFER_SIZE 1024U
#define CAVEMAN_CAVE_TALK_HEADER_SIZE 3U

typedef enum
{
    CAVEMAN_CAVE_TALK_RECEIVE_HEADER,
    CAVEMAN_CAVE_TALK_RECEIVE_PAYLOAD
} CavemanCaveTalk_Receive_t;

static uint8_t                   CavemanCaveTalk_Buffer[CAVEMAN_CAVE_TALK_BUFFER_SIZE];
static CavemanCaveTalk_Receive_t CavemanCaveTalk_LastReceive = CAVEMAN_CAVE_TALK_RECEIVE_HEADER;

static void CavemanCaveTalk_RxCallback(Bsp_Uart_t *const uart);
static CaveTalk_Error_t CavemanCaveTalk_Send(const void *const data, const size_t size);
static CaveTalk_Error_t CavemanCaveTalk_Receive(void *const data, const size_t size, size_t *const bytes_received);
static CaveTalk_Error_t CavemanCaveTalk_Available(size_t *const bytes_available);
static CaveTalk_Error_t CavemanCaveTalk_ConvertBspError(const Bsp_Error_t bsp_error);

CaveTalk_Handle_t CavemanCaveTalk_Handle = {
    .link_handle = {
        .send      = CavemanCaveTalk_Send,
        .receive   = CavemanCaveTalk_Receive,
        .available = CavemanCaveTalk_Available,
    },
    .buffer           = CavemanCaveTalk_Buffer,
    .buffer_size      = sizeof(CavemanCaveTalk_Buffer),
    .listen_callbacks = kCaveTalk_ListenCallbacksNull,
};

Bsp_Error_t CavemanCaveTalk_Start(void)
{
    Bsp_Error_t error = BspUart_Start(BSP_UART_USER_COMMS, NULL, CavemanCaveTalk_RxCallback);

    if (BSP_ERROR_NONE == error)
    {
        CavemanCaveTalk_LastReceive = CAVEMAN_CAVE_TALK_RECEIVE_HEADER;
        error                       = BspUart_StartReceive(BSP_UART_USER_COMMS, CAVEMAN_CAVE_TALK_HEADER_SIZE);
    }

    return error;
}

static CaveTalk_Error_t CavemanCaveTalk_Send(const void *const data, const size_t size)
{
    return CavemanCaveTalk_ConvertBspError(BspUart_StartTransmit(BSP_UART_USER_COMMS, data, size));
}

static CaveTalk_Error_t CavemanCaveTalk_Receive(void *const data, const size_t size, size_t *const bytes_received)
{
    return CavemanCaveTalk_ConvertBspError(BspUart_Read(BSP_UART_USER_COMMS, data, size, bytes_received));
}

static CaveTalk_Error_t CavemanCaveTalk_Available(size_t *const bytes_available)
{
    return CavemanCaveTalk_ConvertBspError(BspUart_Available(BSP_UART_USER_COMMS, bytes_available));
}

static void CavemanCaveTalk_RxCallback(Bsp_Uart_t *const uart)
{
    uint8_t length = CAVEMAN_CAVE_TALK_HEADER_SIZE;

    switch (CavemanCaveTalk_LastReceive)
    {
    case CAVEMAN_CAVE_TALK_RECEIVE_HEADER:
        CavemanCaveTalk_LastReceive = CAVEMAN_CAVE_TALK_RECEIVE_PAYLOAD;
        length                      = *(uint8_t*)((uint32_t)uart->rx_buffer.buffer +
                                                  (uint32_t)((uint32_t)uart->rx_buffer.unlocked * uart->rx_buffer.half_buffer_size)
                                                  + (uint32_t)uart->rx_buffer.write_count[uart->rx_buffer.unlocked] - 1U);
        (void)BspUart_StartReceive(BSP_UART_USER_COMMS, (size_t)length);
        break;
    case CAVEMAN_CAVE_TALK_RECEIVE_PAYLOAD:
        CavemanCaveTalk_LastReceive = CAVEMAN_CAVE_TALK_RECEIVE_HEADER;
        (void)BspUart_StartReceive(BSP_UART_USER_COMMS, (size_t)length);
        break;
    default:
        break;
    }
}

static CaveTalk_Error_t CavemanCaveTalk_ConvertBspError(const Bsp_Error_t bsp_error)
{
    CaveTalk_Error_t cavetalk_error = CAVE_TALK_ERROR_NONE;

    /* BSP UART functions should not return BSP_ERROR_VALUE */
    if (BSP_ERROR_NULL == bsp_error)
    {
        cavetalk_error = CAVE_TALK_ERROR_NULL;
    }
    else if ((BSP_ERROR_HAL == bsp_error) || (BSP_ERROR_PERIPHERAL == bsp_error))
    {
        cavetalk_error = CAVE_TALK_ERROR_SOCKET_CLOSED;
    }
    else if ((BSP_ERROR_BUSY == bsp_error) || (BSP_ERROR_TIMEOUT == bsp_error))
    {
        cavetalk_error = CAVE_TALK_ERROR_INCOMPLETE;
    }

    return cavetalk_error;
}