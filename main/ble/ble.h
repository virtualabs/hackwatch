#ifndef __INC_BLE_H
#define __INC_BLE_H

#define BLESCAN_DEVICES_MAX  10

#include "esp_log.h"
#include "nvs_flash.h"

/* BLE */
#include "esp_nimble_hci.h"
#include "esp_event.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"


#define BLE_DEVICE_NAME_MAXSIZE   32
#define BLE_FRESHNESS_DEFAULT     50
#define BLE_CTRL_EVENT            "BLECTRL"
#define BLE_DEVICE_FOUND          0x0000
#define BLE_DEVICE_VERSION        0x0001
#define BLE_DEVICE_DISCONNECTED   0x0002

typedef struct {
  uint16_t id;
  char *psz_name;
  char *psz_short_name;
} ble_company_id_t;

typedef enum {
  BLE_OFF,
  BLE_IDLE,
  BLE_SCANNER,
  BLE_FINGERPRINT
} ble_controller_mode_t;

typedef enum {
  UNKNOWN,
  PHONE,
  APPLE,
  FITBIT,
  SMARTLOCK,
  KEYFOB,
  HEARTBEAT,
  SMARTWATCH,
  SOUND
} ble_device_type_t;

typedef struct {
  uint8_t record_type;
  uint8_t *pattern;
  uint8_t pattern_size;
} ble_device_fp_pattern_t;

typedef struct {
  ble_device_type_t device_type;
  ble_device_fp_pattern_t *patterns;
} ble_device_fingerprint_t;

typedef struct tBleDevice {
  
  /* Device type. */
  ble_device_type_t device_type;

  /* Device Bluetooth address. */
  ble_addr_t address;

  /* Device name. */
  char psz_name[BLE_DEVICE_NAME_MAXSIZE];

  /* Device rssi. */
  uint8_t i8_rssi;

  /* Freshness. */
  uint8_t freshness;

} ble_device_t;

typedef struct tBleDeviceVersion {
  uint8_t  ble;
  uint16_t company;
  uint16_t software;
} ble_device_version_t;

typedef struct tBleController {
  
  /* Controller mode. */
  ble_controller_mode_t mode;

  /* Custom event loop. */
  bool evt_loop_initialized;
  esp_event_loop_args_t evt_loop_args;
  esp_event_loop_handle_t evt_loop_handle;

  /* Mutex */
  SemaphoreHandle_t mutex;

  /* Connection control. */
  uint16_t conn_handle;
  bool b_connected;

  /* Version info. */
  uint8_t version_ble;
  uint16_t version_compid;
  uint16_t version_soft;

} ble_controller_t;

/* Helpers. */
char *ble_get_version_str(int version);
ble_company_id_t *ble_get_company_info(uint16_t comp_id);

/* Initialisation. */
void ble_ctrl_init(void);
int ble_scan_start(void);
int ble_scan_active(void);
int ble_scan_stop(void);

int ble_connect(ble_device_t *device);
int ble_disconnect(void);

int ble_get_devices_nb(void);
ble_device_t *ble_get_device(int index);
esp_err_t ble_ctrl_event_handler_register(
  int32_t event_id,
  esp_event_handler_t event_handler,
  void* event_handler_arg
);

uint8_t ble_device_get_version(void);
uint16_t ble_device_get_compid(void);
uint16_t ble_device_get_soft(void);

void ble_enter_critical_section(void);
void ble_leave_critical_section(void);

#endif /* __INC_BLE_H */