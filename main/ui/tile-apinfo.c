#include "tile-apinfo.h"

#define BLUE  RGB(0x5,0x9,0xf)
#define TAG "[tile::wifi::apinfo]"

static modal_t apinfo_modal;
static widget_label_t title_lbl, mac_lbl, channel_lbl, security_lbl, rssi_lbl;
static widget_label_t mac_address, channel, security, rssi;
static widget_frame_t frame;
static wifi_ap_t ap_info;
static widget_button_t deauth_btn;

volatile char *psz_mac[18];
volatile char *psz_channel[4];
volatile char *psz_rssi[6];

volatile bool deauth_enabled = false;

char *auth_mode[] = {
  "OPEN",
  "WEP",
  "WPA_PSK",
  "WPA2_PSK",
  "WPA_WPA2_PSK",
  "WPA2_ENTERPRISE",
  "WPA3_PSK",
  "WPA2_WPA3_PSK",
  "WAPI_PSK"
};

int deauth_toggle(widget_t *p_widget)
{
  if (!deauth_enabled)
  {
    /* Set target. */
    wifi_deauth_target(ap_info.bssid, ap_info.channel);

    /* Change button text. */
    widget_button_set_text(&deauth_btn, "Stop");

    /* Deauth is on. */
    deauth_enabled = true;
  }
  else
  {
    /* Stop deauth. */
    wifi_set_mode(WIFI_OFF);

    /* Set button text. */
    widget_button_set_text(&deauth_btn, "Deauth");

    /* Deauth is off. */
    deauth_enabled = false;
  }
  return 0;
}

int apinfo_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_EXIT:
      {
        if (deauth_enabled)
        {
          deauth_enabled = false;
          
          /* Stop deauth. */
          //wifi_set_mode(WIFI_SCANNER);
          //wifi_set_mode(WIFI_OFF);
        }
      }
      break;

    default:
      break;
  }
  return 0;
}

modal_t *modal_apinfo_init(void)
{
  /* Initialize our wifi_ap_info. */
  psz_mac[0] = '\0';
  psz_channel[0] = '\0';
  psz_rssi[0] = '\0';

  /* Initialize our modal tile. */
  modal_init(&apinfo_modal, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  modal_set_border_color(&apinfo_modal, RGB(0,0,0));

  /* Set our own event handler. */
  tile_set_event_handler(&apinfo_modal.tile, apinfo_event_handler);

  /* Add labels. */
  widget_label_init(&title_lbl, &apinfo_modal.tile, 10, 5, 230, 45, "");
  
  widget_label_init(&mac_lbl, &apinfo_modal.tile, 10, 50, 230, 45, "MAC");
  widget_label_set_fontsize(&mac_lbl, LABEL_FONT_SMALL);
  widget_set_front_color((widget_t *)&mac_lbl, BLUE);

  widget_label_init(&mac_address, &apinfo_modal.tile, 70, 50, 160, 45, "");
  widget_label_set_fontsize(&mac_address, LABEL_FONT_SMALL);

  widget_label_init(&channel_lbl, &apinfo_modal.tile, 10, 70, 230, 45, "Channel");
  widget_label_set_fontsize(&channel_lbl, LABEL_FONT_SMALL);
  widget_set_front_color((widget_t *)&channel_lbl, BLUE);

  widget_label_init(&channel, &apinfo_modal.tile, 70, 70, 160, 45, "");
  widget_label_set_fontsize(&channel, LABEL_FONT_SMALL);

  widget_label_init(&security_lbl, &apinfo_modal.tile, 10, 90, 230, 45, "Security");
  widget_label_set_fontsize(&security_lbl, LABEL_FONT_SMALL);
  widget_set_front_color((widget_t *)&security_lbl, BLUE);

  widget_label_init(&security, &apinfo_modal.tile, 70, 90, 160, 45, "");
  widget_label_set_fontsize(&security, LABEL_FONT_SMALL);

  widget_label_init(&rssi_lbl, &apinfo_modal.tile, 10, 110, 230, 45, "RSSI");
  widget_label_set_fontsize(&rssi_lbl, LABEL_FONT_SMALL);
  widget_set_front_color((widget_t *)&rssi_lbl, BLUE);


  widget_label_init(&rssi, &apinfo_modal.tile, 70, 110, 160, 45, "");
  widget_label_set_fontsize(&rssi, LABEL_FONT_SMALL);

  /* Add deauth button. */
  widget_button_init(&deauth_btn, &apinfo_modal.tile, (240 - 120)/2, 175, 120, 50, "Deauth");
  widget_set_bg_color(&deauth_btn.widget, RGB(0x1, 0x2, 0x3));
  widget_set_border_color(&deauth_btn.widget, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&deauth_btn, deauth_toggle);

  /* Add a frame. */
  widget_frame_init(&frame, &apinfo_modal.tile, 5, 45, 230, 185);
  widget_set_border_color(&frame.widget, BLUE);

  /* Return our modal. */
  return &apinfo_modal;
}

void modal_apinfo_set_ap(wifi_ap_t *p_wifi_ap)
{
  /* Copy our AP info into our local structure (cache). */
  memcpy(&ap_info, p_wifi_ap, sizeof(wifi_ap_t));

  /* Copy ESSID. */
  widget_label_set_text(&title_lbl, (char *)ap_info.essid);

  /* Format BSSID. */
  snprintf(
    (char *)psz_mac,
    18,
    "%02x:%02x:%02x:%02x:%02x:%02x",
    p_wifi_ap->bssid[0],
    p_wifi_ap->bssid[1],
    p_wifi_ap->bssid[2],
    p_wifi_ap->bssid[3],
    p_wifi_ap->bssid[4],
    p_wifi_ap->bssid[5]
  );
  widget_label_set_text(&mac_address, (char *)  psz_mac);

  /* Format channel. */
  itoa(p_wifi_ap->channel, (char *)psz_channel, 10);
  widget_label_set_text(&channel, (char *)psz_channel);

  /* Display auth mode. */
  widget_label_set_text(&security, auth_mode[p_wifi_ap->auth_mode]);

  /* Format RSSI. */
  itoa(p_wifi_ap->rssi, (char *)psz_rssi, 10);
  widget_label_set_text(&rssi, (char *)psz_rssi);
}
