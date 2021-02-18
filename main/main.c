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
/*
#include "skull.h"
#include "cat.h"
#include "doh.h"
*/
#include "cat.h"

#include "ui/ui.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/image.h"
#include "ui/listbox.h"
#include "ui/progress.h"
#include "font/font16.h"
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

#if 0
typedef enum {
  SCANNER_IDLE,
  SCANNER_RUNNING
} scanner_state_t;
#endif

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


static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
  {
    printf("[wifi] scan done.\r\n");
    g_state = SCANNER_IDLE;
  }
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

  #if 0
  g_state = SCANNER_IDLE;
  
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
    WIFI_EVENT,
    ESP_EVENT_ANY_ID,
    &event_handler,
    NULL,
    &instance_any_id
  ));
  #endif

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

void main_ui(void *parameter)
{
  widget_image_t image,image2;
  image_t *cat;
  tile_t main_tile, cat_tile, network_tile;
  widget_label_t wifi0, wifi1, wifi2;
  widget_listbox_t list;
  widget_button_t btn;
  widget_progress_t progress;


  /* Load cat image. */
  cat = load_image(img_cat);

  tile_init(&main_tile, NULL);
  tile_set_drawfunc(&main_tile, draw_wifi_stats);

  tile_init(&cat_tile, NULL);
  //widget_image_init(&image, &cat_tile, 10, 10, 200, 200, cat);
  widget_image_init(&image2, NULL, 5, 125, 200, 20, cat);

  tile_init(&network_tile, NULL);
  widget_label_init(&wifi0, NULL, 0, 0, 200, 32, "M");
  widget_label_init(&wifi1, NULL, 0, 0, 200, 32, "Tagadatsointsoin");
  widget_label_init(&wifi2, NULL, 0, 0, 200, 32, "Hackme!");

  widget_progress_init(&progress, NULL, 5, 150, 150, 20);
  widget_progress_set_value(&progress, 25);

  /* Test container. */
  widget_button_init(&btn, NULL, 5, 0, 120, 800,"Test");
  widget_listbox_init(&list, &network_tile, 21, 20, 200, 200);
  widget_listbox_add(&list, (widget_t *)&wifi0);
  widget_listbox_add(&list, (widget_t *)&wifi1);
  widget_listbox_add(&list, (widget_t *)&wifi2);
  widget_listbox_add(&list, (widget_t *)&btn);
  widget_listbox_add(&list, (widget_t *)&progress);

  widget_listbox_remove(&list, (widget_t *)&wifi2);

  tile_link_right(&cat_tile, &network_tile);
  tile_link_right(&network_tile, &main_tile);
  tile_link_right(&main_tile, wifiscan_get_tile());
  
  ui_select_tile(&cat_tile);

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
  //twatch_pmu_audio_power(true);
  //twatch_audio_init(SOUND_DEFAULT_SAMPLE_RATE);

  twatch_pmu_screen_power(true);
  vTaskDelay(200/portTICK_RATE_MS);
  ret = st7789_init();
  assert(ret == ESP_OK);
  st7789_backlight_set(1000);


  /* Initialize WiFi controller. */
  wifi_ctrl_init();
  wifi_set_mode(WIFI_SNIFFER);

  /* Initialize wifi scanner. */
  wifiscan_init();

  //xTaskCreate(wifi_test, "wifi_test", 10000, NULL, 1, NULL);
  xTaskCreate(main_ui, "main_ui", 10000, NULL, 1, NULL);

  
}
