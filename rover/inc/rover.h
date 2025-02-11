#ifndef ROVER_H
#define ROVER_H

typedef double Rover_Meter_t;
typedef double Rover_MetersPerSecond_t;
typedef double Rover_Radian_t;

typedef enum
{
    ROVER_ERROR_NONE,
    ROVER_ERROR_NULL,
    ROVER_ERROR_BSP
} Rover_Error_t;

#endif /* ROVER_H */