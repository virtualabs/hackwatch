
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "wifiscan.h"


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


/**
 * wscan_widget_listitem_event_handler()
 * 
 * @brief: Wifi scan widget listitem event handler.
 * @param event: UI event.
 * @param x: X screen coordinate
 * @param y: Y screen coordinate
 * @param velocity: swipe speed (not used).
 * @return: WE_PROCESSED if event has been processed, WE_ERROR otherwise.
 **/

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
          /* Mark list item as selected. */
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


/**
 * wscan_widget_listitem_drawfunc()
 * 
 * @brief: Wifiscan listitem widget drawing function.
 * @param p_widget: pointer to a `widget_t` structure.
 **/

int wscan_widget_listitem_drawfunc(widget_t *p_widget)
{
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
  /* Success. */
  return TE_PROCESSED;
}


/**
 * wscan_widget_listitem_init()
 * 
 * @brief: Wifiscan listitem widget initialization.
 * @param p_listitem: pointer to a `wifiscan_widget_listitem_t` structure
 * @param p_ap: pointer to a `wifi_ap_t` structure.
 **/

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


/**
 * wscan_aplist_update_handler()
 * 
 * @brief: Wifi scanner callback
 * @param arg: pointer to a `wifiscan_t` structure 
 * @param event_base: ESP32 event base information
 * @param event_id: ESP32 event ID
 * @param event_data: pointer to an array of `wifi_aplist_t` structure
 **/

static void wscan_aplist_update_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  int list_nb_aps, i;
  wifiscan_t *p_wifiscan = (wifiscan_t *)arg;
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

      if ((list_nb_aps > p_wifiscan->nb_aps) && (p_wifiscan->nb_aps < WSCAN_MAX_APS))
      {
        p_wifiscan->aps[list_nb_aps-1].p_ap = p_ap_record;
        widget_listbox_add(&p_wifiscan->listbox, (widget_t *)&p_wifiscan->aps[list_nb_aps-1]);
        p_wifiscan->nb_aps++;
      }
      else
      {
        p_wifiscan->aps[list_nb_aps-1].p_ap = p_ap_record;
      }
    }
    while ((p_ap_record = wifi_aplist_enum_next(p_ap_record)) != NULL);
  }

  /* Remove old access points (no more detected). */
  if ((p_wifiscan->nb_aps > list_nb_aps) && (list_nb_aps > 0))
  {
    for (i=list_nb_aps; i<p_wifiscan->nb_aps; i++)
    {
      ESP_LOGI(TAG, "Remove item %s", p_wifiscan->aps[i].p_ap->essid);
      widget_listbox_remove(&p_wifiscan->listbox, (widget_t *)&p_wifiscan->aps[i]);
    }

    /* Update number of aps. */
    p_wifiscan->nb_aps = list_nb_aps;
  }
}


int wscan_listbox_event_hook(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  wifiscan_t *p_wifiscan = (wifiscan_t *)p_widget;

  /* Only capture the LB_ITEM_SELECTED event. */
  if (event == LB_ITEM_SELECTED)
  {
    /* Forward to our custom event handler. */
    if (p_wifiscan->pfn_event_handler != NULL)
    {
      p_wifiscan->pfn_event_handler(WS_EVENT_SELECTED);
    }
  }

  /* Forward event to our listbox. */
  return p_wifiscan->pfn_event_hook(p_widget, event, x, y, velocity);
}

/**
 * wifiscan_init()
 * 
 * @brief: Initialize a wifiscan widget
 * @param p_wifiscan: pointer to a `wifiscan_t` structure
 * @param p_tile: pointer to a `tile_t` structure (parent tile)
 * @param x: widget X coordinate
 * @param y: widget Y coordinate
 * @param width: widget width
 * @param height: widget height
 **/

void wifiscan_init(wifiscan_t *p_wifiscan, tile_t *p_tile, int x, int y, int width, int height)
{
  int i;

  /* Add a listbox inside this tile. */
  widget_listbox_init(
    (widget_listbox_t *)p_wifiscan,
    p_tile,
    x,
    y,
    width,
    height
  );

  /* Set listbox border color. */
  widget_set_border_color(&p_wifiscan->listbox.widget, RGB(0x0, 0x8, 0xc));
  
  /* Set listbox's scrollbar border color. */
  widget_set_border_color(&p_wifiscan->listbox.scrollbar.widget, RGB(0x0, 0x8, 0xc));


  p_wifiscan->pfn_event_hook = widget_set_eventhandler((widget_t *)p_wifiscan, wscan_listbox_event_hook);

  /* Register a specific handler for scanning events. */
  wifi_ctrl_event_handler_register(
    WIFI_SCANNER_EVENT_APLIST_UPDATED,
    wscan_aplist_update_handler,
    p_wifiscan
  );

  /* Set parameters. */
  p_wifiscan->nb_aps = 0;

  /* Initialize all of our labels. */
  for (i=0; i<WSCAN_MAX_APS; i++)
  {
    wscan_widget_listitem_init(
      &p_wifiscan->aps[i],
      NULL
    );
  }
}


/**
 * wifiscan_get_selected()
 * 
 * @brief: Retrieve the selected AP.
 * @param p_wifiscan: pointer to a `wifiscan_t` structure
 * @return: pointer to a `wifi_ap_t` structure, or NULL if none has been selected.
 **/

wifi_ap_t *wifiscan_get_selected(wifiscan_t *p_wifiscan)
{
  int i;

  for (i=0; i<p_wifiscan->nb_aps; i++)
  {
    if (p_wifiscan->aps[i].b_selected)
    {
      /* AP selected returned. */
      return p_wifiscan->aps[i].p_ap;
    }
  }

  /* No AP selected. */
  return NULL;
}


/**
 * wifiscan_set_event_handler()
 * 
 * @brief: Set wifiscan event handler.
 * @param p_wifiscan: pointer to a `wifiscan_t` structure
 * @param pfn_event_handler: pointer to a FWifiscanEventHandler callback function.
 **/

void wifiscan_set_event_handler(wifiscan_t *p_wifiscan, FWifiscanEventHandler pfn_event_handler)
{
  p_wifiscan->pfn_event_handler = pfn_event_handler;
}