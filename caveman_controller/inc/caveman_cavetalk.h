#ifndef CAVEMAN_CAVETALK_H
#define CAVEMAN_CAVETALK_H

#include <stddef.h>

#include "bsp.h"
#include "cave_talk.h"

extern CaveTalk_Handle_t CavemanCaveTalk_Handle;

Bsp_Error_t CavemanCaveTalk_Start(void);

#endif /* CAVEMAN_CAVETALK_H */