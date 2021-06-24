#include "tile-rogueap.h"

tile_t rogueap_tile;
widget_label_t title_lbl, target_lbl, essid_lbl, channel_lbl;
widget_button_t select_btn, startstop_btn;

/* AP selection. */
modal_t ap_select_modal;
wifiscan_t ap_wifiscan;
widget_button_t ap_select_ok_btn;
widget_frame_t ap_frame;
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

void select_btn_onclick(widget_t *p_widget)
{
  /* Activate AP selection dialog box (modal). */
  ui_set_modal(&ap_select_modal);
}

void start_btn_onclick(widget_t *p_widget)
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

void ap_select_modal_onclick(widget_t *p_widget)
{
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
  tile_set_event_handler((tile_t *)&rogueap_tile, rogueap_tile_event_handler);

  /* Add title. */
  widget_label_init(&title_lbl, &rogueap_tile, 3, 3, 200, 40, "Rogue AP");
  widget_set_front_color((widget_t *)&title_lbl, RGB(0x0, 0x8, 0xc));

  /* Add frame. */
  widget_frame_init(&ap_frame, &rogueap_tile, 2, 39, 240-2, 240-39-2);
  widget_set_border_color((widget_t *)&ap_frame, RGB(0x0, 0x8, 0xc));

  /* No AP selected. */
  psz_target_ap[0] = '\0';
  strncpy(psz_target_channel, "Channel: -", 64);
  widget_label_init(&target_lbl, &rogueap_tile, 7, 46, 200, 40, "Target:");
  widget_label_init(&essid_lbl, &rogueap_tile, 7, 90, 200, 40, psz_target_ap);
  widget_label_init(&channel_lbl, &rogueap_tile, 7, 130, 200, 40, psz_target_channel);

  /* Add our buttons. */
  widget_button_init(
    &select_btn,
    &rogueap_tile,
    7,
    180,
    110,
    50,
    "Sel. AP"
  );
  widget_set_border_color((widget_t *)&select_btn, RGB(0x0, 0x8, 0xc));
  widget_set_bg_color((widget_t *)&select_btn, RGB(0x0, 0x8, 0xc));
  widget_button_set_handler(&select_btn, select_btn_onclick);

  widget_button_init(
    &startstop_btn,
    &rogueap_tile,
    10 + (240-15)/2,
    180,
    (240-15)/2,
    50,
    "Start"
  );
  widget_set_border_color((widget_t *)&startstop_btn, RGB(0x0, 0x8, 0xc));
  widget_set_bg_color((widget_t *)&startstop_btn, RGB(0x0, 0x8, 0xc));
  widget_button_set_handler(&startstop_btn, start_btn_onclick);

  /* Initialize our select AP dialog box. */
  modal_init(&ap_select_modal, 0, 0, 240, 238);
  widget_button_init(
    &ap_select_ok_btn,
    &ap_select_modal.tile,
    60,
    180,
    120,
    50, 
    "OK"
  );
  widget_set_border_color((widget_t *)&ap_select_ok_btn, RGB(0x0, 0x8, 0xc));
  widget_set_bg_color((widget_t *)&ap_select_ok_btn, RGB(0x0, 0x8, 0xc));
  widget_button_set_handler(&ap_select_ok_btn, ap_select_modal_onclick);
  wifiscan_init(&ap_wifiscan, &ap_select_modal.tile, 5, 5, 230, 168);

  /* Return our tile. */
  return &rogueap_tile;
}
