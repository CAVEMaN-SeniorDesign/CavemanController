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

static uint8_t CavemanCaveTalk_Buffer[CAVEMAN_CAVE_TALK_BUFFER_SIZE];

static void CavemanCaveTalk_RxCallback(Bsp_Uart_t *const uart);
static CaveTalk_Error_t CavemanCaveTalk_Send(const void *const data, const size_t size);
static CaveTalk_Error_t CavemanCaveTalk_Receive(void *const data, const size_t size, size_t *const bytes_received);
static CaveTalk_Error_t CavemanCaveTalk_Available(size_t *const bytes_available);

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
    return BspUart_Start(BSP_UART_USER_COMMS, NULL, CavemanCaveTalk_RxCallback);
}

static CaveTalk_Error_t CavemanCaveTalk_Send(const void *const data, const size_t size)
{
    CaveTalk_Error_t cavetalk_error = CAVE_TALK_ERROR_NONE;
    Bsp_Error_t      bsp_error      = BspUart_Transmit(BSP_UART_USER_COMMS, data, size);

    /* BspUart_Transmit should not return BSP_ERROR_VALUE */
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

static CaveTalk_Error_t CavemanCaveTalk_Receive(void *const data, const size_t size, size_t *const bytes_received)
{
    CAVE_TALK_UNUSED(data);
    CAVE_TALK_UNUSED(size);
    CAVE_TALK_UNUSED(bytes_received);

    return CAVE_TALK_ERROR_NONE;
}

static CaveTalk_Error_t CavemanCaveTalk_Available(size_t *const bytes_available)
{
    CAVE_TALK_UNUSED(bytes_available);

    return CAVE_TALK_ERROR_NONE;
}

static void CavemanCaveTalk_RxCallback(Bsp_Uart_t *const uart)
{
    BSP_UNUSED(uart);

    /* TODO */
}