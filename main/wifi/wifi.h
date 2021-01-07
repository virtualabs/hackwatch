#ifndef __INC_WIFI_H
#define __INC_WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "wifi_aplist.h"

ESP_EVENT_DECLARE_BASE(WIFI_CTRL_EVENT);

typedef enum {
  SCANNER_IDLE,
  SCANNER_RUNNING
} scanner_state_t;

typedef enum {
  WIFI_DRIVER_OFF,
  WIFI_DRIVER_STA,
  WIFI_DRIVER_AP,
  WIFI_DRIVER_APSTA
} wifi_driver_mode_t;

typedef enum {
  WIFI_OFF,
  WIFI_SCANNER,
  WIFI_AP,
  WIFI_STA
} wifi_controller_mode_t;

typedef enum {
  WIFI_SCANNER_EVENT_SCAN_DONE = 0,
  WIFI_SCANNER_EVENT_APLIST_UPDATED
} wifi_controller_event;

typedef struct tWifiController {
  /* Wifi ESP32 */
  wifi_driver_mode_t driver_mode;
  wifi_scan_config_t scan_config;
  bool b_enabled;

  /* WiFi controller mode. */
  wifi_controller_mode_t mode;

  /* WiFi controller AP list. */
  wifi_aplist_t ap_list;
  scanner_state_t scan_state;

  /* Custom event loop. */
  esp_event_loop_args_t evt_loop_args;
  esp_event_loop_handle_t evt_loop_handle;

} wifi_controller_t;

/* Initialization. */
void wifi_ctrl_init(void);
void wifi_set_mode(wifi_controller_mode_t mode);

/* Event handling. */
esp_err_t wifi_ctrl_event_handler_register(
  int32_t event_id,
  esp_event_handler_t event_handler,
  void* event_handler_arg
);



#endif /* __INC_WIFI_H */