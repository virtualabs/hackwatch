#include "tile-bleinfo.h"

#define BLUE  RGB(0x5,0x9,0xf)
#define TAG "[tile::ble::bleinfo]"

static modal_t bleinfo_modal;
static widget_label_t title_lbl, ble_ver_lbl, ble_ver_value_lbl, ble_vendor_lbl;
static widget_label_t ble_vendor_value_lbl, ble_ver_stack_lbl, ble_ver_stack_value_lbl;
static widget_label_t ble_software_lbl, ble_software_value_lbl;
static widget_label_t error_lbl;
static widget_spinner_t wait_spinner;
static widget_button_t ok_btn;
static widget_frame_t frame;
static char software_version[8];
static char bd_address[18];

int close_modal(widget_t *p_widget)
{
  int res;
  ESP_LOGI(TAG, "Disconnecting from device");

  /* Disconnect. */
  res = ble_disconnect();
  ESP_LOGI(TAG, "disconnect=%d", res);

  /* Close modal. */
  ui_unset_modal();
  
  return 0;
}

int bleinfo_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_EXIT:
      {
        ESP_LOGI(TAG, "Disconnecting from device");
        /* Disconnect. */
        ble_disconnect();
      }
      break;

    default:
      break;
  }
  return 0;
}

modal_t *modal_bleinfo_init(void)
{
  /* Initialize our BLE device info. */

  /* Initialize our modal tile. */
  modal_init(&bleinfo_modal, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  modal_set_border_color(&bleinfo_modal, RGB(0,0,0));

  /* Set our own event handler. */
  tile_set_event_handler(&bleinfo_modal.tile, bleinfo_event_handler);

  /* Add labels. */
  widget_label_init(&title_lbl, &bleinfo_modal.tile, 10, 5, 230, 45, "");
  
  /* Spinner (hidden by default). */
  widget_spinner_init(&wait_spinner, &bleinfo_modal.tile, 10, 52, 220, 150);
  widget_set_visible(&wait_spinner, WIDGET_HIDDEN);

  /* First label: BLE version. */
  widget_label_init(&ble_ver_lbl, &bleinfo_modal.tile, 10, 52, 80, 15, "BLE Ver.:");
  widget_label_set_fontsize(&ble_ver_lbl, LABEL_FONT_SMALL);
  widget_set_front_color(&ble_ver_lbl.widget, RGB(0x1, 0x2, 0x3));
  widget_label_init(&ble_ver_value_lbl, &bleinfo_modal.tile, 90, 52, 80, 20, "");
  widget_label_set_fontsize(&ble_ver_value_lbl, LABEL_FONT_SMALL);

  /* Second label: Vendor name. */
  widget_label_init(&ble_vendor_lbl, &bleinfo_modal.tile, 10, 72, 80, 15, "Vendor. :");
  widget_label_set_fontsize(&ble_vendor_lbl, LABEL_FONT_SMALL);
  widget_set_front_color(&ble_vendor_lbl.widget, RGB(0x1, 0x2, 0x3));
  widget_label_init(&ble_vendor_value_lbl, &bleinfo_modal.tile, 90, 72, 80, 20, "");
  widget_label_set_fontsize(&ble_vendor_value_lbl, LABEL_FONT_SMALL);

  /* Third label: software version. */
  widget_label_init(&ble_software_lbl, &bleinfo_modal.tile, 10, 92, 80, 15, "Softw.  :");
  widget_label_set_fontsize(&ble_software_lbl, LABEL_FONT_SMALL);
  widget_set_front_color(&ble_software_lbl.widget, RGB(0x1, 0x2, 0x3));
  widget_label_init(&ble_software_value_lbl, &bleinfo_modal.tile, 90, 92, 80, 20, "");
  widget_label_set_fontsize(&ble_software_value_lbl, LABEL_FONT_SMALL);

  
  /* Add OK button. */
  widget_button_init(&ok_btn, &bleinfo_modal.tile, (240 - 120)/2, 175, 120, 50, "Ok");
  widget_set_bg_color(&ok_btn.widget, RGB(0x1, 0x2, 0x3));
  widget_set_border_color(&ok_btn.widget, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&ok_btn, close_modal);

  /* Add a frame. */
  widget_frame_init(&frame, &bleinfo_modal.tile, 5, 45, 230, 185);
  widget_set_border_color(&frame.widget, BLUE);

  /* Return our modal. */
  return &bleinfo_modal;
}

void modal_bleinfo_set_title(ble_device_t *p_device)
{
  if (p_device->psz_name[0])
  {
    widget_label_set_text(&title_lbl, p_device->psz_name);  
  }
  else
  {
    snprintf(
      bd_address,
      18,
      "%02x:%02x:%02x:%02x:%02x:%02x",
      p_device->address.val[5],
      p_device->address.val[4],
      p_device->address.val[3],
      p_device->address.val[2],
      p_device->address.val[1],
      p_device->address.val[0]
    );
    widget_label_set_text(&title_lbl, bd_address);
  }
  
}

void modal_bleinfo_update(uint8_t ble_ver, uint16_t comp_id, uint16_t sw_ver)
{
  ESP_LOGI(TAG, "ble_ver=%d, comp_id=%04x, sw_ver=%04x", ble_ver, comp_id, sw_ver);

  char *version = ble_get_version_str(ble_ver);
  ble_company_id_t *vendor = ble_get_company_info(comp_id);

  /* Hide spinner. */
  widget_set_visible(&wait_spinner, WIDGET_HIDDEN);

  /* Show all labels, set spinner visible. */
  widget_set_visible(&ble_ver_lbl, WIDGET_SHOW);
  widget_set_visible(&ble_ver_value_lbl, WIDGET_SHOW);
  widget_set_visible(&ble_vendor_lbl, WIDGET_SHOW);
  widget_set_visible(&ble_vendor_value_lbl, WIDGET_SHOW);
  widget_set_visible(&ble_software_lbl, WIDGET_SHOW);
  widget_set_visible(&ble_software_value_lbl, WIDGET_SHOW);

  if (version != NULL)
  {
    widget_label_set_text(&ble_ver_value_lbl, version);
  }

  if (vendor != NULL)
  {
    widget_label_set_text(&ble_vendor_value_lbl, vendor->psz_name);
  }

  snprintf(software_version, 8, "%d.%d", (sw_ver & 0xFF00)>>8, (sw_ver & 0xFF));
  widget_label_set_text(&ble_software_value_lbl, software_version);

}

void modal_bleinfo_clear(void)
{
  widget_label_set_text(&ble_ver_value_lbl, "");
  widget_label_set_text(&ble_vendor_value_lbl, "");
  widget_label_set_text(&ble_software_value_lbl, "");
}

void modal_bleinfo_wait(void)
{
  /* Hide all labels, set spinner visible. */
  widget_set_visible(&ble_ver_lbl, WIDGET_HIDDEN);
  widget_set_visible(&ble_ver_value_lbl, WIDGET_HIDDEN);
  widget_set_visible(&ble_vendor_lbl, WIDGET_HIDDEN);
  widget_set_visible(&ble_vendor_value_lbl, WIDGET_HIDDEN);
  widget_set_visible(&ble_software_lbl, WIDGET_HIDDEN);
  widget_set_visible(&ble_software_value_lbl, WIDGET_HIDDEN);

  /* Show spinner. */
  widget_set_visible(&wait_spinner, WIDGET_SHOW);
}