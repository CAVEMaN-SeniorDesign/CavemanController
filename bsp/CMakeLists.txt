# STM32 HAL
if(NOT BOARD)
    set(BOARD "CAVeBoard")
endif()
message("Board: " ${BOARD})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}/cmake/stm32cubemx)

################################################################################
# BSP library
################################################################################
set(BSP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(BSP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(BSP_SRCS
    ${BSP_SRC_DIR}/bsp.c
    ${BSP_SRC_DIR}/bsp_encoder.c
    ${BSP_SRC_DIR}/bsp_encoder_user.c
    ${BSP_SRC_DIR}/bsp_logger.c
    ${BSP_SRC_DIR}/bsp_logger_user.c
    ${BSP_SRC_DIR}/bsp_motor.c
    ${BSP_SRC_DIR}/bsp_pwm.c
    ${BSP_SRC_DIR}/bsp_pwm_user.c
    ${BSP_SRC_DIR}/bsp_servo.c
    ${BSP_SRC_DIR}/bsp_tick.c
    ${BSP_SRC_DIR}/bsp_tick_user.c
    ${BSP_SRC_DIR}/bsp_uart.c
    ${BSP_SRC_DIR}/bsp_uart_user.c
)

set_source_files_properties(
    ${BSP_SRCS}
    PROPERTIES
    COMPILE_OPTIONS "-Werror"
)

add_library(Bsp INTERFACE)

target_compile_definitions(Bsp
    INTERFACE
        $<$<CONFIG:Debug>:BSP_LOGGER>
        $<$<CONFIG:Debug>:BSP_LOGGER_USER_CUSTOM_LOGGER>
        $<$<STREQUAL:${BOARD},"CAVeBoard">:BOARD_CAVEBOARD>
        $<$<STREQUAL:${BOARD},"Nucleo-64">:BOARD_NUCLEO_64>
        $<$<STREQUAL:${BOARD},"STM32-E407">:BOARD_STM32_E407>
)

target_include_directories(Bsp
    INTERFACE
        ${BSP_INC_DIR}
)

target_sources(Bsp
    INTERFACE
        ${BSP_SRCS}
)

target_link_libraries(Bsp
    INTERFACE
        stm32cubemx
)