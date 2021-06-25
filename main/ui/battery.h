#ifndef __INC_BATTERY_H
#define __INC_BATTERY_H

#include "ui/ui.h"
#include "ui/widget.h"

#define   BATTERY_WIDTH   30
#define   BATTERY_HEIGHT  18

typedef struct {
  widget_t widget;

  /* Update delay. */
  int delay;

  /* Battery percentage. */
  int percentage;
  bool b_usb_connected;

} widget_batt_t;

void widget_battery_init(widget_batt_t *p_battery, tile_t *p_tile, int x, int y);

#endif /* __INC_BATTERY_H */