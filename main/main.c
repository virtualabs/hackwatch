#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "drivers/bma423/bma.h"
#include "drivers/st7789.h"
#include <math.h>
#include "twatch.h"
#include "img.h"
#include "digits.h"

#include "ui/ui.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/image.h"
#include "ui/listbox.h"
#include "ui/progress.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "wifi/wifi.h"
#include "wifi/wifiscan.h"

#define I2S_NUM 0
#define SAMPLE_RATE 44100
#define BUFSIZE (4096*2)

#define DEFAULT_SCAN_LIST_SIZE 10
#define WIFI_MAX_CHANNELS 14
#define WIFI_MAX_MEASURES 5 

/* Define our viewer settings. */
#define VIEWER_X_POS 22
#define VIEWER_Y_POS 70
#define VIEWER_X_SCALE 14
#define VIEWER_Y_SCALE 1
#define VIEWER_WIDTH 196
#define VIEWER_HEIGHT 100

#define TAG "wifi_scanner"


bool g_deauth_active = false;


volatile scanner_state_t g_state = SCANNER_IDLE;
int8_t channels_measures[WIFI_MAX_CHANNELS][WIFI_MAX_MEASURES];

void init_measures(void)
{
  int i,j;

  for (i=0; i<WIFI_MAX_CHANNELS; i++)
    for (j=0; j<WIFI_MAX_MEASURES; j++)
      channels_measures[i][j] = -100;
}

void insert_measure(int channel, int8_t rssi)
{
  int i;
  for (i=1; i<WIFI_MAX_MEASURES; i++)
    channels_measures[channel][i-1] = channels_measures[channel][i];
  channels_measures[channel][WIFI_MAX_MEASURES-1] = rssi;
}

int8_t get_average(int channel)
{
  int avg = 0;
  int i;

  for (i=0; i<WIFI_MAX_MEASURES; i++)
    avg += channels_measures[channel][i];

  return (int8_t)(avg/WIFI_MAX_MEASURES);
}


void disp_ssid(uint8_t *ssid)
{
  if (strlen((char *)ssid) > 0)
    printf(" SSID: %s\r\n", ssid);
  else
    printf("  SSID: [NULL]\r\n");
}

void disp_mac(uint8_t *mac)
{
  int i;

  printf(" BSSID: ");
  for (i=0; i<6; i++)
    printf((i<5)?"%02x:":"%02x", mac[i]);
  printf("\r\n");
}

/**
 * AP list update event handler.
 **/

static void aplist_update_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  int rssi;
  wifi_aplist_t *p_list = (wifi_aplist_t *)event_data;
  wifi_ap_t *p_ap_record;

  ESP_LOGI(TAG, "aplist_update_handler triggered");

  ESP_LOGD(TAG, "starting aplist enumeration ...");
  p_ap_record = wifi_aplist_enum_first(p_list);

  if (p_ap_record != NULL)
  {
    do
    {
      ESP_LOGD(TAG, "updating rssi for AP %s", p_ap_record->essid);
  
      /* Update channel signal strength. */
      rssi = p_ap_record->rssi;
      if (rssi > 0)
        rssi = 0;
      else if (rssi < -100)
        rssi = -100;

      insert_measure(p_ap_record->channel, rssi);
    }
    while ((p_ap_record = wifi_aplist_enum_next(p_ap_record)) != NULL);
  }

  ESP_LOGD(TAG, "enumeration done ...");
}



void wifi_test(void *parameter)
{
  wifi_scan_config_t wifi_config;
  uint16_t number = DEFAULT_SCAN_LIST_SIZE;
  wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
  uint16_t ap_count = 0;
  int j,k;
  int8_t rssi;

  /* Enable our wifi scanner. */
  wifi_set_mode(WIFI_SCANNER);

  wifi_ctrl_event_handler_register(
    WIFI_SCANNER_EVENT_APLIST_UPDATED,
    aplist_update_handler,
    NULL
  );

  /* reset channels RSSI. */
  init_measures();

  k=0;
  while (1)
  {

    if (g_state == SCANNER_IDLE)
    {
      esp_wifi_scan_start(&wifi_config, false);
      g_state = SCANNER_RUNNING;
    }

    /* Get APs */
    #if 0
      ap_count = DEFAULT_SCAN_LIST_SIZE;
      if (esp_wifi_scan_get_ap_records(&ap_count, ap_info) == ESP_OK)
      {
        k++;
        for (j=0; j<ap_count; j++)
        {
          /* Update channel signal strength. */
          rssi = ap_info[j].rssi;
          if (rssi > 0)
            rssi = 0;
          else if (rssi < -100)
            rssi = -100;

          insert_measure(ap_info[j].primary, rssi);
        }
      }
    #endif

    vTaskDelay(10);
  }
}

int draw_wifi_stats(tile_t *p_tile)
{
  int i;
  int8_t rssi;

  /* Read channel stats and display a nice graph. */
   tile_draw_line(
    p_tile,
    VIEWER_X_POS-1,
    VIEWER_Y_POS-1,
    VIEWER_X_POS + VIEWER_WIDTH + 1,
    VIEWER_Y_POS-1,
    RGB(0x0, 0x0,0xf)
  );
  tile_draw_line(
    p_tile,
    VIEWER_X_POS + VIEWER_WIDTH + 1,
    VIEWER_Y_POS-1,
    VIEWER_X_POS + VIEWER_WIDTH + 1,
    VIEWER_Y_POS + VIEWER_HEIGHT + 1,
    RGB(0x0, 0x0,0xf)
  );
  tile_draw_line(
    p_tile,
    VIEWER_X_POS - 1,
    VIEWER_Y_POS + VIEWER_HEIGHT + 1,
    VIEWER_X_POS + VIEWER_WIDTH + 1,
    VIEWER_Y_POS + VIEWER_HEIGHT + 1,
    RGB(0x0, 0x0,0xf)
  );
  tile_draw_line(
    p_tile,
    VIEWER_X_POS-1,
    VIEWER_Y_POS + VIEWER_HEIGHT + 1,
    VIEWER_X_POS-1,
    VIEWER_Y_POS-1,
    RGB(0x0, 0x0,0xf)
  );


  /* Draw bars. */
  for (i=0; i<WIFI_MAX_CHANNELS; i++)
  {
    rssi = get_average(i);
    tile_fill_region(
      p_tile,
      i*VIEWER_X_SCALE + VIEWER_X_POS,
      (VIEWER_HEIGHT + VIEWER_Y_POS) - (100 + rssi)*VIEWER_Y_SCALE,
      VIEWER_X_SCALE,
      (100 + rssi)*VIEWER_Y_SCALE,
      RGB(0xf,0xf,0xf) 
    );
  }

  return 0;
}

void deauth_go(widget_t *p_widget)
{
  widget_button_t *p_button = (widget_button_t *)(p_widget->p_user_data);
  wifi_ap_t *p_ap;

  if (!g_deauth_active)
  {
    p_ap = wifiscan_get_selected();
    if (p_ap != NULL)
    {
      /* Start deauth. */
      wifi_deauth_target(p_ap->bssid, p_ap->channel);

      /* Set button name to "STOP" */
      widget_button_set_text(p_button, "STOP");
      g_deauth_active = true;
    }
  }
  else
  {
    /* Rename button. */
    widget_button_set_text(p_button, "Deauth !");
    
    /* Restart scanner. */
    wifi_set_mode(WIFI_SCANNER);
  }
}

void main_ui(void *parameter)
{
  tile_t main_tile;

  /* Deauth tool */
  tile_t deauth_screen;
  widget_button_t btn_deauth;

  /* Main screen */
  tile_init(&main_tile, NULL);
  tile_set_drawfunc(&main_tile, draw_wifi_stats);

  /* Deauth tool. */
  tile_init(&deauth_screen, NULL);
  wifiscan_init(&deauth_screen, 5, 5, 230, 190);
  widget_button_init(&btn_deauth, &deauth_screen, 5,200,230,35,"Deauth !");
  widget_button_set_handler(&btn_deauth, deauth_go);

  ui_select_tile(&deauth_screen);

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
