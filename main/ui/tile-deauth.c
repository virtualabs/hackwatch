#include "tile-deauth.h"

tile_t deauth_tile;
widget_label_t deauth_title_lbl, deauth_target_lbl, deauth_essid_lbl, deauth_channel_lbl;
widget_button_t deauth_select_btn, deauth_startstop_btn;

/* AP selection. */
modal_t deauth_select_modal;
wifiscan_t deauth_wifiscan;
widget_button_t deauth_select_ok_btn;
widget_frame_t deauth_frame;
wifi_ap_t *deauth_target = NULL;
char psz_deauth_target[64];
char psz_target_channel[64];


int deauth_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
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

int deauth_select_btn_onclick(widget_t *p_widget)
{
  /* Activate AP selection dialog box (modal). */
  ui_set_modal(&deauth_select_modal);

  /* Success. */
  return TE_PROCESSED;
}

int deauth_start_btn_onclick(widget_t *p_widget)
{
  if (wifi_get_mode() != WIFI_DEAUTH)
  {
    if (deauth_target != NULL)
    {
      printf("Starting Deauth for %s\r\n", deauth_target->essid);
      widget_button_set_text(&deauth_startstop_btn, "Stop");
      wifi_deauth_target(deauth_target->bssid, deauth_target->channel);
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

    widget_button_set_text(&deauth_startstop_btn, "Start");
  }
  /* Success. */
  return TE_PROCESSED;
}

int deauth_select_modal_onclick(widget_t *p_widget)
{
  /* Do we have a selected AP ? */
  deauth_target = wifiscan_get_selected(&deauth_wifiscan);
  if (deauth_target != NULL)
  {
    /* Update target ap info. */
    strncpy(psz_deauth_target, (char *)deauth_target->essid, 64);
    snprintf(psz_target_channel, 64, "Channel: %d", deauth_target->channel);
  }
  else
  {
    /* No AP selected. */
    strncpy(psz_deauth_target, "", 64);
    strncpy(psz_target_channel, "Channel: -", 64);
  }

  /* Deactivate modal. */
  ui_set_modal(NULL);

  /* Success. */
  return TE_PROCESSED;
}

tile_t *tile_deauth_init(void)
{
  /* Initialize our tile. */
  tile_init(&deauth_tile, NULL);
  tile_set_event_handler(&deauth_tile, deauth_tile_event_handler);

  /* Add title. */
  widget_label_init(&deauth_title_lbl, &deauth_tile, 3, 3, 200, 40, "Deauth");
  widget_set_front_color((widget_t *)&deauth_title_lbl, RGB(0xf,0xf,0xf));

  /* Add frame. */
  widget_frame_init(&deauth_frame, &deauth_tile, 2, 39, 240-2, 240-39-2);
  widget_set_border_color((widget_t *)&deauth_frame, RGB(0x1, 0x2, 0x3));

  /* No AP selected. */
  psz_deauth_target[0] = '\0';
  strncpy(psz_target_channel, "Channel: -", 64);
  widget_label_init(&deauth_target_lbl, &deauth_tile, 7, 46, 200, 40, "Target:");
  widget_label_init(&deauth_essid_lbl, &deauth_tile, 7, 90, 200, 40, psz_deauth_target);
  widget_label_init(&deauth_channel_lbl, &deauth_tile, 7, 130, 200, 40, psz_target_channel);

  /* Add our buttons. */
  widget_button_init(
    &deauth_select_btn,
    &deauth_tile,
    7,
    180,
    110,
    50,
    "Sel. AP"
  );
  widget_set_border_color((widget_t *)&deauth_select_btn, RGB(0x1, 0x2, 0x3));
  widget_set_bg_color((widget_t *)&deauth_select_btn, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&deauth_select_btn, deauth_select_btn_onclick);

  widget_button_init(
    &deauth_startstop_btn,
    &deauth_tile,
    10 + (240-15)/2,
    180,
    (240-15)/2,
    50,
    "Start"
  );
  widget_set_border_color((widget_t *)&deauth_startstop_btn, RGB(0x1, 0x2, 0x3));
  widget_set_bg_color((widget_t *)&deauth_startstop_btn, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&deauth_startstop_btn, deauth_start_btn_onclick);

  /* Initialize our select AP dialog box. */
  modal_init(&deauth_select_modal, 0, 0, 240, 238);
  widget_button_init(
    &deauth_select_ok_btn,
    &deauth_select_modal.tile,
    60,
    180,
    120,
    50, 
    "OK"
  );
  widget_set_border_color((widget_t *)&deauth_select_ok_btn, RGB(0x1, 0x2, 0x3));
  widget_set_bg_color((widget_t *)&deauth_select_ok_btn, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&deauth_select_ok_btn, deauth_select_modal_onclick);
  wifiscan_init(&deauth_wifiscan, &deauth_select_modal.tile, 5, 5, 230, 168);

  /* Return our tile. */
  return &deauth_tile;
}