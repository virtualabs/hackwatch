#ifndef __INC_HACKWATCH_BLESCAN_H
#define __INC_HACKWATCH_BLESCAN_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/button.h"
#include "../ble/ble.h"
#include "tile-bleinfo.h"

tile_t *tile_blescan_init(void);

/* BleDevice ListItem widget. */
typedef struct {

  /* Base widget. */
  widget_t widget;

  /* Device infos */
  ble_device_t *p_device;

  /* Selected. */
  bool b_selected;
} blescan_widget_listitem_t;


void blescan_widget_listitem_init(blescan_widget_listitem_t *p_listitem, ble_device_t *p_device);
void blescan_widget_listitem_update(blescan_widget_listitem_t *p_listitem, ble_device_t *p_device);

#endif /* __INC_HACKWATCH_BLESCAN_H */