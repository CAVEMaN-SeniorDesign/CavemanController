cmake_minimum_required(VERSION 3.30)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
message("Build type: " ${CMAKE_BUILD_TYPE})

# # Include toolchain file
# include("${CMAKE_SOURCE_DIR}/toolchain.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Project with support for ASM and C languages
project(CavemanController LANGUAGES C ASM)

# CAVeTalk library
include(${CMAKE_SOURCE_DIR}/CAVeTalk.cmake)

################################################################################
# BSP library
################################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/bsp)
get_target_property(BSP_USER_SRCS BspUser INTERFACE_SOURCES)
set_source_files_properties(
    ${BSP_USER_SRCS}
    PROPERTIES
    COMPILE_OPTIONS "-Werror"
)
get_target_property(BSP_SRCS Bsp INTERFACE_SOURCES)
set_source_files_properties(
    ${BSP_SRCS}
    PROPERTIES
    COMPILE_OPTIONS "-Werror"
)

################################################################################
# Rover config library
################################################################################
get_target_property(ROVER_CONFIG_SRCS RoverConfig INTERFACE_SOURCES)
set_source_files_properties(
    ${ROVER_CONFIG_SRCS}
    PROPERTIES
    COMPILE_OPTIONS "-Werror"
)

################################################################################
# LSM6DSV16X driver
################################################################################
set(LSM6DSV16X_DIR ${CMAKE_SOURCE_DIR}/external/lsm6dsv16x-pid)
add_library(lsm6dsv16x INTERFACE)
target_sources(lsm6dsv16x
    INTERFACE
        ${LSM6DSV16X_DIR}/lsm6dsv16x_reg.c
)
target_include_directories(lsm6dsv16x
    INTERFACE
        ${LSM6DSV16X_DIR}
)

################################################################################
# CAVEMAN controller firmware
################################################################################
set(ROVER_DIR ${CMAKE_SOURCE_DIR}/rover)
set(ROVER_INC_DIR ${ROVER_DIR}/inc)
set(ROVER_SRC_DIR ${ROVER_DIR}/src)

set(ROVER_SRCS
    ${ROVER_SRC_DIR}/rover.c
    ${ROVER_SRC_DIR}/rover_4ws.c
    ${ROVER_SRC_DIR}/rover_camera.c
    ${ROVER_SRC_DIR}/rover_imu.c
    ${ROVER_SRC_DIR}/rover_pid.c
)

set(CAVEMAN_CONTROLLER_DIR ${CMAKE_SOURCE_DIR}/caveman_controller)
set(CAVEMAN_CONTROLLER_INC_DIR ${CAVEMAN_CONTROLLER_DIR}/inc)
set(CAVEMAN_CONTROLLER_SRC_DIR ${CAVEMAN_CONTROLLER_DIR}/src)

set(CAVEMAN_CONTROLLER_SRCS
    ${CAVEMAN_CONTROLLER_SRC_DIR}/caveman_buttons.c
    ${CAVEMAN_CONTROLLER_SRC_DIR}/caveman_controller.c
    ${CAVEMAN_CONTROLLER_SRC_DIR}/caveman_cavetalk.c
    ${CAVEMAN_CONTROLLER_SRC_DIR}/caveman_dust_sensor.c
    ${CAVEMAN_CONTROLLER_SRC_DIR}/caveman_gas_sensor.c
)

set_source_files_properties(
    ${CAVEMAN_CONTROLLER_SRCS}
    ${ROVER_SRCS}
    PROPERTIES
    COMPILE_OPTIONS "-Werror"
)

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CAVEMAN_CONTROLLER_INC_DIR}
        ${ROVER_INC_DIR}
)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${CAVEMAN_CONTROLLER_SRCS}
        ${ROVER_SRCS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Bsp
        RoverConfig
        CAVeTalk-c
        lsm6dsv16x
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_SIZE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf"
        COMMAND ${CMAKE_OBJCOPY} -O ihex "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
        COMMAND ${CMAKE_OBJCOPY} -O binary -S "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
)

################################################################################
# CI and tools
################################################################################
set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
set(CPPCHECK_SOURCES ${BSP_SRCS} ${BSP_USER_SRCS} ${ROVER_SRCS} ${ROVER_CONFIG_SRCS} ${CAVEMAN_CONTROLLER_SRCS})
include(${TOOLS_DIR}/cppcheck/cppcheck.cmake)
get_target_property(BSP_USER_INC_DIRS BspUser INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(BSP_INC_DIRS Bsp INTERFACE_INCLUDE_DIRECTORIES)
set(UNCRUSTIFY_SOURCES ${CPPCHECK_SOURCES})
set(UNCRUSTIFY_INC_DIRS ${BSP_INC_DIRS} ${BSP_USER_INC_DIRS} ${ROVER_INC_DIR} ${ROVER_CONFIG_INC_DIR} ${CAVEMAN_CONTROLLER_INC_DIR})
include(${TOOLS_DIR}/uncrustify/uncrustify.cmake)