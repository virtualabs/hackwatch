#include "tile-scanner.h"
#include "tile-apinfo.h"

tile_t scanner_tile;

#undef TAG
#define TAG "[tile::wifi::scanner]"

wifiscan_t g_wifiscan;
widget_label_t lbl_scanner;
modal_t *p_apinfo_modal;


void scanner_wifi_event_handler(wifiscan_event_t event)
{
  wifi_ap_t *p_ap;

  if (event == WS_EVENT_SELECTED)
  {
    /* Process selected AP. */
    p_ap = wifiscan_get_selected(&g_wifiscan);
    ESP_LOGI(TAG, "AP selected: %s", p_ap->essid);

    /* Display our modal. */
    modal_apinfo_set_ap(p_ap);
    ui_set_modal(p_apinfo_modal);
  }
}

int scanner_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Enable scanner. */
        wifi_set_mode(WIFI_SCANNER);

        /* Disable eco mode. */
        disable_ecomode();
      }
      break;

    case TE_EXIT:
      {
        /* Stop scanner. */
        wifi_set_mode(WIFI_OFF);

        /* Re-enable eco mode. */
        enable_ecomode();
      }
      break;

    default:
      break;
  }

  /* Success. */
  return TE_PROCESSED;
}


tile_t *tile_scanner_init(void)
{
  /* Initialize our modal box. */
  p_apinfo_modal = modal_apinfo_init();

  /* Initialize our tile. */
  tile_init(&scanner_tile, NULL);
  tile_set_event_handler(&scanner_tile, scanner_tile_event_handler);

  /* Initialize our title label */
  widget_label_init(&lbl_scanner, &scanner_tile, 3, 3, 237, 36, "WiFi Networks");

  /* Add a wifiscan widget. */
  wifiscan_init(&g_wifiscan, &scanner_tile, 5, 39, 230, 196);
  wifiscan_set_event_handler(&g_wifiscan, scanner_wifi_event_handler);

  /* Return our tile. */
  return &scanner_tile;
}