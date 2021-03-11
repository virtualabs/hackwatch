#ifndef __INC_WIFISCAN_H
#define __INC_WIFISCAN_H

#include "esp_log.h"

#include "ui/ui.h"
#include "ui/listbox.h"
#include "ui/label.h"
#include "wifi.h"

#define TAG "[wifiscan]"

#define WSCAN_LISTBOX_X 10
#define WSCAN_LISTBOX_Y 10
#define WSCAN_LISTBOX_W 220
#define WSCAN_LISTBOX_H 220

#define WSCAN_LISTITEM_HEIGHT 60

#define WSCAN_RSSI_LOW  -90
#define WSCAN_RSSI_LOW_COLOR  RGB(15, 0, 0)
#define WSCAN_RSSI_MEDIUM  -60
#define WSCAN_RSSI_MEDIUM_COLOR  RGB(14, 6, 0)
#define WSCAN_RSSI_GOOD_COLOR  RGB(1, 7, 0)


#define WSCAN_MAX_APS   20

typedef enum {
  WS_EVENT_SELECTED
} wifiscan_event_t;

/* Event callback type. */
typedef void (*FWifiscanEventHandler)(wifiscan_event_t event);

/* AccessPoint ListItem widget. */
typedef struct {

  /* Base widget. */
  widget_t widget;

  /* AP infos */
  wifi_ap_t *p_ap;

  /* Selected. */
  bool b_selected;
} wifiscan_widget_listitem_t;


/* Wifiscan widget structure. */
typedef struct {
  widget_listbox_t listbox;
  
  tile_t tile_scanner;
  wifiscan_widget_listitem_t aps[WSCAN_MAX_APS];
  int nb_aps;

  /* Listbox event hook. */
  FEventHandler pfn_event_hook;

  /* Event callback. */
  FWifiscanEventHandler pfn_event_handler;

} wifiscan_t;

void wifiscan_init(wifiscan_t *p_wifiscan, tile_t *p_tile, int x, int y, int width, int height);
wifi_ap_t *wifiscan_get_selected(wifiscan_t *p_wifiscan);
void wifiscan_set_event_handler(wifiscan_t *p_wifiscan, FWifiscanEventHandler pfn_event_handler);

#endif /* __INC_WIFISCAN_H */