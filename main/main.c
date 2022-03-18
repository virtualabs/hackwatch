#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "menu.h"
#include "twatch.h"
#include "hal/hal.h"
#include "ui/tile-scanner.h"
#include "ui/tile-blescan.h"
#include "ui/tile-apinfo.h"
#include "ui/tile-clock.h"
#include "ui/tile-settings.h"
#include "ui/tile-channels.h"
#include "ui/tile-rogueap.h"
#include "ui/tile-tvbgone.h"

/* Include WiFi icon if needed. */
#ifdef CONFIG_INCLUDE_WIFI
  #include "img/wifi_icon.h"
#endif

/* Include WiFi icon if needed. */
#ifdef CONFIG_INCLUDE_BLE
  #include "ble/ble.h"
  #include "img/bluetooth_icon.h"
#endif


/* Include IR icon if needed. */
#ifdef CONFIG_INCLUDE_IR
  #include "img/ir_icon.h"
#endif

#include "img/settings_icon.h"
/* #include "img/bluetooth_icon.h" */


void main_ui(void *parameter)
{
  tile_t main_tile;
  tile_t settings_tile /*, bluetooth_tile*/;
    
  tile_t *p_tile_first, *p_tile_current; /* main menu */
  tile_t *p_sub_first, *p_sub_current; /* submenu */

  tile_t *p_settings_one_tile, *p_settings_two_tile, *p_settings_three_tile;
  image_t *settings;
  
  widget_image_t settings_img;
  widget_label_t settings_lbl;
  widget_label_t label_main;
  
  #ifdef CONFIG_INCLUDE_WIFI
    tile_t wifi_tile;
    image_t *wifi;
    widget_image_t wifi_img;
    widget_label_t wifi_lbl;
  #endif /* CONFIG_INCLUDE_WIFI */

  #ifdef CONFIG_INCLUDE_BLE
    tile_t bluetooth_tile;
    image_t *bluetooth;
    widget_image_t bluetooth_img;
    widget_label_t bluetooth_lbl;
  #endif /* CONFIG_INCLUDE_BLE */


  #ifdef CONFIG_INCLUDE_IR
    tile_t ir_tile;
    image_t *ir;
    widget_image_t ir_img;
    widget_label_t ir_lbl;
  #endif

  /* Main screen */
  tile_init(&main_tile, NULL);
  widget_label_init(&label_main, &main_tile, 80, 110, 220, 45, "Main tile");

  /* For the next release. 
    tile_init(&bluetooth_tile, NULL);
    bluetooth = load_image(bluetooth_icon);
    widget_image_init(&bluetooth_img, &bluetooth_tile, 80, (240-88)/2 - 20, 88, 88, bluetooth);
    widget_label_init(&bluetooth_lbl, &bluetooth_tile, 60, 150, 120, 50, "Bluetooth");
  */

  p_settings_one_tile = tile_settings_one_init();
  p_settings_two_tile = tile_settings_two_init();
  p_settings_three_tile = tile_settings_three_init();
  
  /* Main circular "menu" */
  p_tile_first = NULL;
  p_tile_current = NULL;

    /* Add clock tile (mandatory). */
  p_tile_current = menu_add_menu(p_tile_current, tile_clock_init());
  p_tile_first = p_tile_current;

  /* Add WiFi menu (if enabled). */
  #ifdef CONFIG_INCLUDE_WIFI
    /* WiFi screen */
    tile_init(&wifi_tile, NULL);
    wifi = load_image(wifi_icon);
    widget_image_init(&wifi_img, &wifi_tile, 70, (240-88)/2 - 20, 100, 88, wifi);
    widget_label_init(&wifi_lbl, &wifi_tile, 90, 150, 120, 50, "WiFi");
    p_tile_current = menu_add_menu(p_tile_current, &wifi_tile);
  #endif

  /* Add Bluetooth Low Energy screen (if enabled). */
  #ifdef CONFIG_INCLUDE_BLE
    tile_init(&bluetooth_tile, NULL);
    bluetooth = load_image(bluetooth_icon);
    widget_image_init(&bluetooth_img, &bluetooth_tile, 80, (240-88)/2 - 20, 88, 88, bluetooth);
    widget_label_init(&bluetooth_lbl, &bluetooth_tile, 60, 150, 120, 50, "Bluetooth");
    p_tile_current = menu_add_menu(p_tile_current, &bluetooth_tile);
  #endif /* CONFIG_INCLUDE_BLE */


  /* Add IR menu (if enabled). */
  #ifdef CONFIG_INCLUDE_IR
    /* Add IR menu. */
    tile_init(&ir_tile, NULL);
    ir = load_image(ir_icon);
    widget_image_init(&ir_img, &ir_tile, 70, (240-88)/2 - 20, 100, 88, ir);
    widget_label_init(&ir_lbl, &ir_tile, 100, 150, 120, 50, "IR");
    p_tile_current = menu_add_menu(p_tile_current, &ir_tile);
  #endif

  /* Add Settings menu. */
  tile_init(&settings_tile, NULL);
  settings = load_image(settings_icon);
  widget_image_init(&settings_img, &settings_tile, 80, (240-88)/2 - 20, 88, 88, settings);
  widget_label_init(&settings_lbl, &settings_tile, 70, 150, 120, 50, "Settings");
  p_tile_current = menu_add_menu(p_tile_current, &settings_tile);

  /* Loop on first tile. */
  tile_link_right(p_tile_current, p_tile_first);

  /* With bluetooth menu, for next release
  tile_link_right(&settings_tile, &bluetooth_tile);
  tile_link_right(&bluetooth_tile, p_clock_tile);
  */

 #ifdef CONFIG_INCLUDE_BLE
    /* BLE "submenu" */
    p_sub_first = NULL;
    p_sub_current = NULL;
    
    #ifdef CONFIG_BLE_SCANNER
      p_sub_current = menu_add_tile(&bluetooth_tile, p_sub_current, tile_blescan_init);
      if (p_sub_first == NULL)
        p_sub_first = p_sub_current;
    #endif
    
    if (p_sub_current != p_sub_first)
      tile_link_right(p_sub_current, p_sub_first);
    tile_link_bottom(&bluetooth_tile, p_sub_first);
 #endif 

  #ifdef CONFIG_INCLUDE_WIFI
    /* Wifi "submenu" */
    p_sub_first = NULL;
    p_sub_current = NULL;

    #ifdef CONFIG_WIFI_SCANNER
      p_sub_current = menu_add_tile(&wifi_tile, p_sub_current, tile_scanner_init);
      if (p_sub_first == NULL)
        p_sub_first = p_sub_current;
    #endif

    #ifdef CONFIG_WIFI_CHANNELS
      p_sub_current = menu_add_tile(&wifi_tile, p_sub_current, tile_channels_init);
      if (p_sub_first == NULL)
        p_sub_first = p_sub_current;
    #endif

    #ifdef CONFIG_WIFI_ROGUEAP
      p_sub_current = menu_add_tile(&wifi_tile, p_sub_current, tile_rogueap_init);
      if (p_sub_first == NULL)
        p_sub_first = p_sub_current;
    #endif

    if (p_sub_current != p_sub_first)
      tile_link_right(p_sub_current, p_sub_first);
    tile_link_bottom(&wifi_tile, p_sub_first);
  #endif

  
  #ifdef CONFIG_INCLUDE_IR
    /* IR "submenu" */
    tile_link_bottom(&ir_tile, tile_tvbgone_init());
  #endif

  /* Settings "submenu" */
  tile_link_bottom(&settings_tile, p_settings_three_tile);
  tile_link_bottom(&settings_tile, p_settings_two_tile);
  tile_link_bottom(&settings_tile, p_settings_one_tile);
  tile_link_right(p_settings_one_tile, p_settings_two_tile);
  tile_link_right(p_settings_two_tile, p_settings_three_tile);
  tile_link_right(p_settings_three_tile, p_settings_one_tile);

  /* Select our main tile. */
  ui_select_tile(p_tile_first);

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
  esp_log_level_set("*", ESP_LOG_DEBUG);

  /* Init HAL. */
  twatch_hal_init();
  
  ui_init();
  
  #ifdef CONFIG_INCLUDE_WIFI
  /* Initialize WiFi controller. */
  wifi_ctrl_init();
  #endif
  
  #ifdef CONFIG_INCLUDE_BLE
  /* Initialize BLE controller. */
  ble_ctrl_init();
  #endif

  /* Start UI in a dedicated task. */
  xTaskCreate(main_ui, "main_ui", 10000, NULL, 1, NULL);
}
