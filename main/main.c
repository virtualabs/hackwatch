#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include "twatch.h"
#include "ui/tile-scanner.h"
#include "ui/tile-apinfo.h"


void main_ui(void *parameter)
{
  tile_t main_tile;
  tile_t wifi_tile;
  tile_t *deauth_tile, *apinfo_tile;
  
  widget_label_t label_main;
  widget_label_t label_wifi;

  /* Main screen */
  tile_init(&main_tile, NULL);
  widget_label_init(&label_main, &main_tile, 80, 110, 220, 45, "Main tile");

  /* Main screen */
  tile_init(&wifi_tile, NULL);
  widget_label_init(&label_wifi, &wifi_tile, 80, 110, 220, 45, "WiFi tile");


  deauth_tile = tile_scanner_init();
  apinfo_tile = tile_apinfo_init();

  tile_link_right(&main_tile, &wifi_tile);
  tile_link_left(&main_tile, &wifi_tile);
  tile_link_bottom(&wifi_tile, deauth_tile);
  tile_link_bottom(deauth_tile, apinfo_tile);


  /* Select our main tile. */
  ui_select_tile(&main_tile);

  while (1)
  {
    ui_process_events();
    vTaskDelay(1);
  }
}

void app_main(void)
{
  esp_err_t ret;

  esp_log_level_set("*", ESP_LOG_INFO);

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

  twatch_vibrate_init();
  twatch_pmu_init();
  twatch_screen_init();

  /* Initialize WiFi controller. */
  wifi_ctrl_init();
  wifi_set_mode(WIFI_SCANNER);

  xTaskCreate(main_ui, "main_ui", 10000, NULL, 1, NULL);
}
