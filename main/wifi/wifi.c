#include <string.h>
#include "wifi.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_event.h"

#define TAG "wifi_ctrl"
#define DEFAULT_SCAN_LIST_SIZE 10

ESP_EVENT_DEFINE_BASE(WIFI_CTRL_EVENT);

wifi_controller_t g_wifi_ctrl;

void wifi_scanner_task(void *parameter);

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
  }
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
  xTaskCreate(wifi_scanner_task, "wifi_scanner", 10000, NULL, 1, NULL);
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

    default:
      break;
  }

  ESP_LOGD(TAG, "current moden is now %d", mode);
  g_wifi_ctrl.mode = mode;

}