#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "twatch.h"
#include "ui/tile-scanner.h"
#include "ui/tile-apinfo.h"
#include "ui/tile-clock.h"
#include "ui/tile-settings.h"
#include "ui/tile-channels.h"
#include "ui/tile-rogueap.h"
#include "ui/tile-tvbgone.h"
#include "img/wifi_icon.h"
#include "img/ir_icon.h"
#include "img/settings_icon.h"
/* #include "img/bluetooth_icon.h" */


void main_ui(void *parameter)
{
  tile_t main_tile;
  tile_t ir_tile;
  tile_t wifi_tile, settings_tile /*, bluetooth_tile*/;
  tile_t *p_scanner_tile, *p_clock_tile, *p_channels_tile;
  tile_t *p_settings_one_tile, *p_settings_two_tile, *p_settings_three_tile;
  tile_t *p_rogue_tile;
  tile_t *p_tvbgone_tile;
  image_t *wifi, *settings, *bluetooth, *ir;
  
  widget_label_t label_main;
  widget_image_t wifi_img, settings_img /*, bluetooth_img*/, ir_img;
  widget_label_t wifi_lbl, settings_lbl /*, bluetooth_lbl*/, ir_lbl;

  /* Main screen */
  tile_init(&main_tile, NULL);
  widget_label_init(&label_main, &main_tile, 80, 110, 220, 45, "Main tile");

  /* IR screen */
  tile_init(&ir_tile, NULL);
  ir = load_image(ir_icon);
  widget_image_init(&ir_img, &ir_tile, 70, (240-88)/2 - 20, 100, 88, ir);
  widget_label_init(&ir_lbl, &ir_tile, 100, 150, 120, 50, "IR");

  /* WiFi screen */
  tile_init(&wifi_tile, NULL);
  wifi = load_image(wifi_icon);
  widget_image_init(&wifi_img, &wifi_tile, 70, (240-88)/2 - 20, 100, 88, wifi);
  widget_label_init(&wifi_lbl, &wifi_tile, 90, 150, 120, 50, "WiFi");

  /* Settings screen */
  tile_init(&settings_tile, NULL);
  settings = load_image(settings_icon);
  widget_image_init(&settings_img, &settings_tile, 80, (240-88)/2 - 20, 88, 88, settings);
  widget_label_init(&settings_lbl, &settings_tile, 70, 150, 120, 50, "Settings");

  /* Bluetooth screen */

  /* For the next release. 
    tile_init(&bluetooth_tile, NULL);
    bluetooth = load_image(bluetooth_icon);
    widget_image_init(&bluetooth_img, &bluetooth_tile, 80, (240-88)/2 - 20, 88, 88, bluetooth);
    widget_label_init(&bluetooth_lbl, &bluetooth_tile, 60, 150, 120, 50, "Bluetooth");
  */

  p_scanner_tile = tile_scanner_init();
  p_clock_tile = tile_clock_init();
  p_channels_tile = tile_channels_init();
  p_settings_one_tile = tile_settings_one_init();
  p_settings_two_tile = tile_settings_two_init();
  p_settings_three_tile = tile_settings_three_init();
  p_rogue_tile = tile_rogueap_init();
  p_tvbgone_tile = tile_tvbgone_init();
  
  /* Main circular "menu" */
  tile_link_right(p_clock_tile, &wifi_tile);
  tile_link_right(&wifi_tile, &ir_tile);
  tile_link_right(&ir_tile, &settings_tile);
  tile_link_right(&settings_tile, p_clock_tile);

  /* With bluetooth menu, for next release
  tile_link_right(&settings_tile, &bluetooth_tile);
  tile_link_right(&bluetooth_tile, p_clock_tile);
  */

  /* Wifi "submenu" */
  tile_link_bottom(&wifi_tile, p_channels_tile);
  tile_link_bottom(&wifi_tile, p_rogue_tile);
  tile_link_bottom(&wifi_tile, p_scanner_tile);
  tile_link_right(p_scanner_tile, p_channels_tile);
  tile_link_right(p_channels_tile, p_rogue_tile);
  tile_link_right(p_rogue_tile, p_scanner_tile);
  
  /* IR "submenu" */
  tile_link_bottom(&ir_tile, p_tvbgone_tile);

  /* Settings "submenu" */
  tile_link_bottom(&settings_tile, p_settings_three_tile);
  tile_link_bottom(&settings_tile, p_settings_two_tile);
  tile_link_bottom(&settings_tile, p_settings_one_tile);
  tile_link_right(p_settings_one_tile, p_settings_two_tile);
  tile_link_right(p_settings_two_tile, p_settings_three_tile);
  tile_link_right(p_settings_three_tile, p_settings_one_tile);

  /* Select our main tile. */
  ui_select_tile(p_clock_tile);

  /* Enable eco mode. */
  enable_ecomode();

  while (1)
  {
    ui_process_events();
    vTaskDelay(1);
  }
}

/**
 * HackWatch main application routine.
 **/

void app_main(void)
{
  esp_log_level_set("*", ESP_LOG_WARN);

  /* Initialize UI. */
  ui_init();

  /* Check power management. */
  if (twatch_pmu_init() == ESP_OK)
  {
    printf("[pmu] PMU is detected\r\n");
  }
  else
  {
    printf("[pmu] PMU is not detected\r\n");
  }

  /* Initialize our twatch-lib. */
  twatch_vibrate_init();
  twatch_screen_init();
  twatch_rtc_init();
  
  /* Initialize WiFi controller. */
  wifi_ctrl_init();
  
  /* Start UI in a dedicated task. */
  xTaskCreate(main_ui, "main_ui", 10000, NULL, 1, NULL);
}
