#ifndef __INC_HACKWATCH_CHANNELS_H
#define __INC_HACKWATCH_CHANNELS_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/button.h"
#include "../wifi/wifi.h"

#define   CHANNELS_SCAN_TIMEOUT_TICKS   10
#define   CHANNELS_MAX_CHANS            14
#define   CHANNELS_NB_MEASURES           5

#define   CHANNELS_COLUMN_WIDTH         16
#define   CHANNELS_COLUMN_HEIGHT       180
#define   CHANNELS_TABLE_X               8
#define   CHANNELS_TABLE_Y              30

tile_t *tile_channels_init(void);

#endif /* __INC_HACKWATCH_CHANNELS_H */