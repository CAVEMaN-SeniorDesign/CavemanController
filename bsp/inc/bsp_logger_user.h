#ifndef BSP_LOGGER_USER_H
#define BSP_LOGGER_USER_H

#include "bsp.h"

void BspLoggerUser_RegisterCustomLogger(void);
void BspLoggerUser_TransmitCallback(Bsp_UartHandle_t* uart_handle);

#endif /* BSP_LOGGER_USER_H */