
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "wifiscan.h"

static wifiscan_t g_wscan;


/**************************************
 * AccessPoint ListItem widget
 *************************************/

uint16_t wscan_get_rssi_color(int rssi)
{
  if (rssi <= WSCAN_RSSI_LOW)
    return WSCAN_RSSI_LOW_COLOR;
  
  if (rssi <= WSCAN_RSSI_MEDIUM)
    return WSCAN_RSSI_MEDIUM_COLOR;

  return WSCAN_RSSI_GOOD_COLOR;
}

int wscan_widget_listitem_event_handler(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  bool b_processed = false;
  wifiscan_widget_listitem_t *p_listitem = (wifiscan_widget_listitem_t *)p_widget->p_user_data;

  if (p_listitem != NULL)
  {
    switch (event)
    {

      case LB_ITEM_SELECTED:
        {
          p_listitem->b_selected = true;
          b_processed = true;
        }
        break;

      case LB_ITEM_DESELECTED:
        {
          p_listitem->b_selected = false;
          b_processed = true;
        }
        break;

      default:
        break;
    }

    /* Event processed (or not) */
    return b_processed;
  }

  /* Event not processed. */
  return WE_ERROR;
}


void wscan_widget_listitem_drawfunc(widget_t *p_widget)
{
  int text_width, dx, dy;
  char bssid[18];
  char channel[4];

  wifiscan_widget_listitem_t *p_listitem = (wifiscan_widget_listitem_t *)p_widget->p_user_data;

  if (p_listitem != NULL)
  {
    if (p_listitem->p_ap != NULL)
    {

      /* Draw background. */
      widget_fill_region(
        p_widget,
        1,
        1,
        p_widget->box.width - 2,
        p_widget->box.height - 2,
        (p_listitem->b_selected)?p_widget->style.front:p_widget->style.background
      );

      /* Draw ESSID. */
      widget_draw_text_x2(
        p_widget,
        5,
        5,
        (char *)p_listitem->p_ap->essid,
        wscan_get_rssi_color(p_listitem->p_ap->rssi)
      );

      /* Draw BSSID. */
      snprintf(
        bssid,
        18,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        p_listitem->p_ap->bssid[0],
        p_listitem->p_ap->bssid[1],
        p_listitem->p_ap->bssid[2],
        p_listitem->p_ap->bssid[3],
        p_listitem->p_ap->bssid[4],
        p_listitem->p_ap->bssid[5]
      );
      widget_draw_text(
        p_widget,
        5,
        5 + 36,
        bssid,
        (p_listitem->b_selected)?p_widget->style.background:p_widget->style.front
      );

      /* Draw BSSID. */
      snprintf(channel, 4, "%d", p_listitem->p_ap->channel);
      widget_draw_text(
        p_widget,
        p_widget->box.width - 5 - 20,
        5 + 36,
        channel,
        (p_listitem->b_selected)?p_widget->style.background:p_widget->style.front
      );
    }
  }
}


void wscan_widget_listitem_init(wifiscan_widget_listitem_t *p_listitem, wifi_ap_t *p_ap)
{
  /* Initialize our widget. */
  widget_init(&p_listitem->widget, NULL, 0, 0, 0, WSCAN_LISTITEM_HEIGHT);

  /* Set parameters. */
  p_listitem->p_ap = p_ap;
  p_listitem->b_selected = false;

  /* Set userdata. */
  widget_set_userdata(&p_listitem->widget, (void *)p_listitem);

  /* Set drawing function. */
  widget_set_drawfunc(&p_listitem->widget, wscan_widget_listitem_drawfunc);

  /* Set default event handler. */
  widget_set_eventhandler(&p_listitem->widget, wscan_widget_listitem_event_handler);
}


static void wscan_aplist_update_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  int list_nb_aps, i;
  wifi_aplist_t *p_list = (wifi_aplist_t *)event_data;
  wifi_ap_t *p_ap_record;

  
  /* Add all scanned access points. */
  list_nb_aps = 0;
  p_ap_record = wifi_aplist_enum_first(p_list);
  if (p_ap_record != NULL)
  {
    do
    {
      ESP_LOGI(
        TAG,
        "AP: [%02x:%02x:%02x:%02x:%02x:%02x] %s (%d) frsh:%d",
        p_ap_record->bssid[0],
        p_ap_record->bssid[1],
        p_ap_record->bssid[2],
        p_ap_record->bssid[3],
        p_ap_record->bssid[4],
        p_ap_record->bssid[5],
        p_ap_record->essid,
        p_ap_record->rssi,
        p_ap_record->freshness
      );

      list_nb_aps++;

      if ((list_nb_aps > g_wscan.nb_aps) && (g_wscan.nb_aps < WSCAN_MAX_APS))
      {
        g_wscan.aps[list_nb_aps-1].p_ap = p_ap_record;
        widget_listbox_add(&g_wscan.listbox, &g_wscan.aps[list_nb_aps-1]);
        g_wscan.nb_aps++;
      }
      else
      {
        g_wscan.aps[list_nb_aps-1].p_ap = p_ap_record;
      }
    }
    while ((p_ap_record = wifi_aplist_enum_next(p_ap_record)) != NULL);
  }

  /* Remove old access points (no more detected). */
  if ((g_wscan.nb_aps > list_nb_aps) && (list_nb_aps > 0))
  {
    for (i=list_nb_aps; i<g_wscan.nb_aps; i++)
    {
      ESP_LOGI(TAG, "Remove item %s", g_wscan.aps[i].p_ap->essid);
      widget_listbox_remove(&g_wscan.listbox, &g_wscan.aps[i]);
    }

    /* Update number of aps. */
    g_wscan.nb_aps = list_nb_aps;
  }
}


void wifiscan_init(void)
{
  int i;

  /* We create a specific tile (view) for this controller. */
  tile_init(&g_wscan.tile_scanner, NULL);

  /* Add a listbox inside this tile. */
  widget_listbox_init(
    &g_wscan.listbox,
    &g_wscan.tile_scanner,
    WSCAN_LISTBOX_X,
    WSCAN_LISTBOX_Y,
    WSCAN_LISTBOX_W,
    WSCAN_LISTBOX_H
  );

  /* Register a specific handler for scanning events. */
  wifi_ctrl_event_handler_register(
    WIFI_SCANNER_EVENT_APLIST_UPDATED,
    wscan_aplist_update_handler,
    NULL
  );

  /* Set parameters. */
  g_wscan.nb_aps = 0;

  /* Initialize all of our labels. */
  for (i=0; i<WSCAN_MAX_APS; i++)
  {
    wscan_widget_listitem_init(
      &g_wscan.aps[i],
      NULL
    );
  }
}


tile_t *wifiscan_get_tile(void)
{
  return (tile_t *)&g_wscan.tile_scanner;
}