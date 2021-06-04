#include "tile-rogueap.h"

tile_t rogueap_tile;
widget_label_t title_lbl, target_lbl, essid_lbl, channel_lbl;
widget_button_t select_btn, startstop_btn;

/* AP selection. */
modal_t ap_select_modal;
wifiscan_t ap_wifiscan;
widget_button_t ap_select_ok_btn;
wifi_ap_t *target_ap = NULL;
char psz_target_ap[64];
char psz_target_channel[64];


int rogueap_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Enable scanner. */
        wifi_set_mode(WIFI_SCANNER);
      }
      break;

    case TE_EXIT:
      {
        /* Stop scanner. */
        wifi_set_mode(WIFI_OFF);
      }
      break;

    default:
      break;
  }

  /* Success. */
  return TE_PROCESSED;
}

void select_btn_onclick(struct widget_t *p_widget)
{
  /* Activate AP selection dialog box (modal). */
  ui_set_modal(&ap_select_modal);
}

void start_btn_onclick(struct widget_t *p_widget)
{
  if (wifi_get_mode() != WIFI_ROGUEAP)
  {
    if (target_ap != NULL)
    {
      printf("Starting rogue AP for %s\r\n", target_ap->essid);
      widget_button_set_text(&startstop_btn, "Stop");
      wifi_rogueap_set_target(target_ap);
    }
    else
    {
      printf("No target AP selected.\r\n");
    }
  }
  else
  {
    /* Enable scanner. */
    wifi_set_mode(WIFI_SCANNER);
    widget_button_set_text(&startstop_btn, "Start");
  }
}

void ap_select_modal_onclick(struct widget_t *p_widget)
{
  wifi_ap_t *ap_selected;

  /* Do we have a selected AP ? */
  target_ap = wifiscan_get_selected(&ap_wifiscan);
  if (target_ap != NULL)
  {
    /* Update target ap info. */
    strncpy(psz_target_ap, (char *)target_ap->essid, 64);
    snprintf(psz_target_channel, 64, "Channel: %d", target_ap->channel);
  }
  else
  {
    /* No AP selected. */
    strncpy(psz_target_ap, "", 64);
    strncpy(psz_target_channel, "Channel: -", 64);
  }

  /* Deactivate modal. */
  ui_set_modal(NULL);
}

tile_t *tile_rogueap_init(void)
{
  /* Initialize our tile. */
  tile_init(&rogueap_tile, NULL);
  tile_set_event_handler(&rogueap_tile, rogueap_tile_event_handler);

  /* Add title. */
  widget_label_init(&title_lbl, &rogueap_tile, 3, 3, 200, 40, "Rogue AP");
  widget_set_front_color(&title_lbl, RGB(0x0, 0x8, 0xc));

  /* No AP selected. */
  psz_target_ap[0] = '\0';
  strncpy(psz_target_channel, "Channel: -", 64);
  widget_label_init(&target_lbl, &rogueap_tile, 3, 46, 200, 40, "Target:");
  widget_label_init(&essid_lbl, &rogueap_tile, 3, 90, 200, 40, psz_target_ap);
  widget_label_init(&channel_lbl, &rogueap_tile, 3, 130, 200, 40, psz_target_channel);

  /* Add our buttons. */
  widget_button_init(
    &select_btn,
    &rogueap_tile,
    5,
    185,
    (240-15)/2,
    50,
    "Sel. AP"
  );
  widget_button_set_handler(&select_btn, select_btn_onclick);

  widget_button_init(
    &startstop_btn,
    &rogueap_tile,
    10 + (240-15)/2,
    185,
    (240-15)/2,
    50,
    "Start"
  );
  widget_button_set_handler(&startstop_btn, start_btn_onclick);

  /* Initialize our select AP dialog box. */
  modal_init(&ap_select_modal, 0, 0, 230, 230);
  widget_button_init(
    &ap_select_ok_btn,
    &ap_select_modal.tile,
    65,
    175,
    120,
    50, 
    "OK"
  );
  widget_button_set_handler(&ap_select_ok_btn, ap_select_modal_onclick);
  wifiscan_init(&ap_wifiscan, &ap_select_modal.tile, 5, 5, 220, 168);

  /* Return our tile. */
  return &rogueap_tile;
}