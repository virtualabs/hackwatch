#ifndef __INC_WIFI_H
#define __INC_WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event.h"
#include "wifi_aplist.h"


typedef struct {
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {

} wifi_ieee80211_probe_resp_t;


typedef void (*POSI_func)(void *p1);

/* 802.11 raw packet feature. */
extern POSI_func *g_osi_funcs_p;
extern void *g_wifi_global_lock;
void *ic_ebuf_alloc( void *p1, int p2, void *p3);
void *ieee80211_post_hmac_tx(void *param_1);
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
esp_err_t ieee80211_raw_frame_sanity_check(void *param_1,void *param_2,void *param_3,void *param_4);

/* Callback types. */
typedef void (*FWifiPacketReceivedCb)(wifi_promiscuous_pkt_t *p_wifi_pkt);


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
  WIFI_SNIFFER,
  WIFI_SCANNER,
  WIFI_DEAUTH,
  WIFI_FAKEAP,
  WIFI_ROGUEAP,
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
  TaskHandle_t current_task_handle;

  /* WiFi controller AP list. */
  wifi_aplist_t ap_list;
  scanner_state_t scan_state;

  /* WiFi deauth */
  uint8_t deauth_target[6];
  int deauth_channel;

  /* WiFi rogueAP */
  wifi_ap_t *p_rogueap_target;

  /* WiFi callback. */
  FWifiPacketReceivedCb pfn_on_packet_received;

  /* Custom event loop. */
  bool evt_loop_initialized;
  esp_event_loop_args_t evt_loop_args;
  esp_event_loop_handle_t evt_loop_handle;

} wifi_controller_t;

/* Initialization. */
void wifi_ctrl_init(void);
void wifi_set_mode(wifi_controller_mode_t mode);
void wifi_set_channel(int channel);
wifi_controller_mode_t wifi_get_mode(void);
void wifi_deauth_target(uint8_t *p_bssid, int channel);
void wifi_rogueap_set_target(wifi_ap_t *p_target);
void wifi_set_sniffer_handler(FWifiPacketReceivedCb callback);

/* Event handling. */
esp_err_t wifi_ctrl_event_handler_register(
  int32_t event_id,
  esp_event_handler_t event_handler,
  void* event_handler_arg
);



#endif /* __INC_WIFI_H */