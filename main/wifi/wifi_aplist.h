#ifndef __INC_WIFI_AP_H
#define __INC_WIFI_AP_H

#define WIFI_APLIST_MAX_NUMBER  10
#define WIFI_AP_FRESHNESS_MAX   255
#define WIFI_AP_FRESHNESS_OLD   100

#include "esp_wifi.h"

typedef struct t_wifi_ap {
  struct t_wifi_ap *p_next;
  uint8_t essid[33];
  uint8_t bssid[6];
  uint8_t channel;
  int8_t rssi;
  wifi_auth_mode_t auth_mode;
  wifi_auth_mode_t pw_cipher;
  wifi_cipher_type_t gp_cipher;
  uint8_t freshness;
} wifi_ap_t;

typedef struct {
  wifi_ap_t *p_first;
  int count;
} wifi_aplist_t;

void wifi_aplist_init(wifi_aplist_t *p_list);
void wifi_aplist_add(wifi_aplist_t *p_list, wifi_ap_record_t *ap);
wifi_ap_t *wifi_aplist_enum_first(wifi_aplist_t *p_list);
wifi_ap_t *wifi_aplist_enum_next(wifi_ap_t *ap);


#endif /* __INC_WIFI_AP_H */