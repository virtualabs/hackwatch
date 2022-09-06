#include "ble.h"
#include "ble_hack.h"

#define TAG "ble_ctrl"

ble_controller_t g_ble_ctrl;
ble_device_t g_devices[BLESCAN_DEVICES_MAX];
volatile int g_devices_nb = 0;

uint8_t g_current_ble_version;
uint16_t g_current_comp_id;
uint16_t g_current_sw_version;

extern void modal_bleinfo_update(uint8_t ble_ver, uint16_t comp_id, uint16_t sw_ver);

char *BLE_VERSIONS[] = {
  "4.0",  /* Starting at version 6. */
  "4.1",
  "4.2",
  "5.0",
  "5.1",
  "5.2",
  "5.3"
};

ble_company_id_t BLE_COMPANY_IDS[] = {
  {0x000A, "Qualcomm", "QCOM"},
  {0x000D, "Texas Instruments", "TI"},
  {0x000F, "Broadcom", "BCOM"},
  {0x0013, "Atmel", "ATMEL"},
  {0x001D, "Qualcomm", "QCOM"},
  {0x0025, "NXP", "NXP"},
  {0x0030, "ST Micro", "STM"},
  {0x0046, "MediaTek", "MTK"},
  {0x004C, "Apple", "APPL"},
  {0x0059, "Nordic", "NORD"},
  {0x005B, "Ralink", "RLNK"},
  {0x0071, "connectBlue", "COBL"},
  {0x0075, "Samsung", "SAMS"},
  {0x00CD, "Microchip", "MICRO"},
  {0x00D7, "Qualcomm", "QCOM"},
  {0x00D8, "Qualcomm", "QCOM"},
  {0x00E0, "Google", "GOOG"},
  {0x010F, "HiSilicon", "HISL"},
  {0x0131, "Cypress", "CYPR"},
  {0x02E5, "Espressif", "ESPF"},

  /* TODO: add other interesting BLE SoC vendors here :) */
  {0xFFFF, NULL, NULL}
};

__attribute__ ((aligned(4)))
ble_device_fp_pattern_t FITBIT_CHARGEHR_FP[] = {
  {0x06, (uint8_t *)"\xba\x56\x89\xa6\xfa\xbf\xa2\xbd\x01\x46\x7d\x6e\x00\xfb\xab\xad", 16},
  {0x09, (uint8_t *)"\x43\x68\x61\x72\x67\x65\x20\x48\x52", 9},
  {0x0a, (uint8_t *)"\xfa", 1},
  {0x16, (uint8_t *)"\x0a\x18\x12", 3},
  {0, NULL, 0}
};

__attribute__ ((aligned(4)))
ble_device_fp_pattern_t FITBIT_CHARGE_FP[] = {
  {0x06, (uint8_t *)"\xba\x56\x89\xa6\xfa\xbf\xa2\xbd\x01\x46\x7d\x6e\x00\xfb\xab\xad", 16},
  {0x09, (uint8_t *)"\x43\x68\x61\x72\x67\x65", 6},
  {0x0a, (uint8_t *)"\xfa", 1},
  {0x16, (uint8_t *)"\x0a\x18\x08", 3},
  {0, NULL, 0}
};

__attribute__ ((aligned(4)))
ble_device_fp_pattern_t FITBIT_CHARGE3_FP[] = {
  {0x06, (uint8_t *)"\xba\x56\x89\xa6\xfa\xbf\xa2\xbd\x01\x46\x7d\x6e\x00\xfb\xab\xad", 16},
  {0x16, (uint8_t *)"\x0a\x18\x1c", 3},
  {0, NULL, 0}
};

__attribute__ ((aligned(4)))
ble_device_fp_pattern_t FITBIT_ONE_FP[] = {
  {0x09, (uint8_t *)(uint8_t *)"\x4f\x6e\x65", 3},
  {0x0a, (uint8_t *)(uint8_t *)"\xfa", 1},
  {0x16, (uint8_t *)(uint8_t *)"\x0a\x18\x05\x04", 4},
  {0, NULL, 0}
};

__attribute__ ((aligned(4)))
ble_device_fp_pattern_t FITBIT_FLEX_FP[] = {
  {0x01, (uint8_t *)"\x06", 1},
  {0x06, (uint8_t *)"\xba\x56\x89\xa6\xfa\xbf\xa2\xbd\x01\x46\x7d\x6e\xa7\x53\xab\xad", 16},
  {0x09, (uint8_t *)"\x46\x6c\x65\x78", 4},
  {0x0a, (uint8_t *)"\xfa", 1},
  {0x16, (uint8_t *)"\x0a\x18\x07\x04", 4},
  {0, NULL, 0}
};

__attribute__ ((aligned(4)))
ble_device_fp_pattern_t FITBIT_SURGE_FP[] = {
  {0x01, (uint8_t *)"\x06", 1},
  {0x06, (uint8_t *)"\xba\x56\x89\xa6\xfa\xbf\xa2\xbd\x01\x46\x7d\x6e\x00\xfb\xab\xad", 16},
  {0x09, (uint8_t *)"\x53\x75\x72\x67\x65", 5},
  {0x16, (uint8_t *)"\x0a\x18\x10", 4},
  {0, NULL, 0}
};

ble_device_fp_pattern_t GARMIN_FENIX3_FP[] = {
  {0x07, (uint8_t *)"\x66\x9a\x0c\x20\x00\x08\x9a\x94\xe3\x11\x7b\x66\x10\x3e\x4e\x6a", 16},
  {0x09, (uint8_t *)"\x66\x65\x6e\x69\x78\x20\x33\x00", 8},
  {0x16, (uint8_t *)"\x10\x3e\x00\x12\x00", 5},
  {0, NULL, 0}
};

ble_device_fp_pattern_t GARMIN_FORERUNNER_FP[] = {
  {0x07, (uint8_t *)"\x66\x9a\x0c\x20\x00\x08\x9a\x94\xe3\x11\x7b\x66\x10\x3e\x4e\x6a", 16},
  {0x09, (uint8_t *)"\x46\x6f\x72\x65\x72\x75\x6e\x6e\x65\x72", 10},
  {0x16, (uint8_t *)"\x10\x3e\x00\x02\x00", 5},
  {0, NULL, 0}
};

ble_device_fp_pattern_t APPLE_WATCH_FP[] = {
  {0x01, (uint8_t *)"\x1a", 1},
  {0xFF, (uint8_t *)"\x4c\x00\x0c\x0e\x00\x5c", 6},
  {0x00, NULL, 0}
};

ble_device_fp_pattern_t APPLE_IPHONE_FP[] = {
  {0x01, (uint8_t *)"\x1a", 1},
  {0xFF, (uint8_t *)"\x4c\x00\x0c\x0e", 4},
  {0x00, NULL, 0}
};

ble_device_fp_pattern_t WITHINGS_ACTIVITE_FP[] = {
  {0x01, (uint8_t *)"\x06", 1},
  {0x09, (uint8_t *)"\x57\x20\x41\x63\x74\x69\x76\x69\x74\x65", 10},
  {0xFF, (uint8_t *)"\x00\x24\xe4\x36", 4},
  {0x00, NULL, 0}
};

ble_device_fp_pattern_t NIKE_FUELBAND_FP[] = {
  {0x01, (uint8_t *)"\x06", 1},
  {0x07, (uint8_t *)"\x66\x9a\x0c\x20\x00\x08\xc1\x81\xe2\x11\xdd\x31\x10\xc4\xcd\x83", 16},
  {0x00, NULL, 0}
};

ble_device_fp_pattern_t JAWBONE_UP2_FP[] = {
  {0x01, (uint8_t *)"\x06", 1},
  {0x06, (uint8_t *)"\xbc\x4f\x45\xf3\x56\x50\xa1\x9c\x11\x41\x80\x45\x00\x10\x1c\x15", 16},
  {0x09, (uint8_t *)"\x55\x50\x32", 3},
  {0x16, (uint8_t *)"\x0a\x18\x02", 3},
  {0x00, NULL, 0}
};

ble_device_fp_pattern_t AICASE_PADLOCK_FP[] = {
  {0x01, (uint8_t *)"\x06", 1},
  {0x09, (uint8_t *)"\x46\x42\x35\x30\x53", 5},
  {0xff, (uint8_t *)"\x48\x53\x7f\x06\x02\x01\x02\x85\x64\x00\x01\x00", 12},
  {0x00, NULL, 0}
};

const ble_device_fingerprint_t BLE_DEVICES_FP[] = {
  {
    SMARTWATCH,
    APPLE_WATCH_FP
  },
  {
    SMARTWATCH,
    WITHINGS_ACTIVITE_FP
  },
  {
    FITBIT,
    FITBIT_CHARGEHR_FP
  },
  {
    FITBIT,
    FITBIT_CHARGE_FP
  },
  {
    FITBIT,
    FITBIT_CHARGE3_FP
  },
  {
    FITBIT,
    FITBIT_ONE_FP
  },
  {
    FITBIT,
    FITBIT_FLEX_FP
  },
  {
    FITBIT,
    FITBIT_SURGE_FP
  },
  {
    SMARTWATCH,
    GARMIN_FENIX3_FP
  },
  {
    SMARTWATCH,
    GARMIN_FORERUNNER_FP
  },
  {
    PHONE,
    APPLE_IPHONE_FP
  },
  {
    HEARTBEAT,
    NIKE_FUELBAND_FP
  },
  {
    HEARTBEAT,
    JAWBONE_UP2_FP
  },
  {
    SMARTLOCK,
    AICASE_PADLOCK_FP
  },
  {
    UNKNOWN,
    NULL
  }
};



void llc_llcp_version_ind_pdu_send(uint16_t conhdl);

void ble_store_config_init(void);

void ble_host_task(void *param)
{
     nimble_port_run(); //This function will return only when nimble_port_stop() is executed.
     nimble_port_freertos_deinit();
}

char *ble_get_version_str(int version)
{
  /* Reserved values. */
  if (version<6)
    return NULL;

  version -= 6;  
  if ((version >= 0) && (version <= (sizeof(BLE_VERSIONS)/sizeof(BLE_VERSIONS[0]))) )
  {
    return BLE_VERSIONS[version];
  }

  /* Unknown version. */
  return NULL;
}

ble_company_id_t *ble_get_company_info(uint16_t comp_id)
{
  int i=0;
  while (BLE_COMPANY_IDS[i].id != 0xFFFF)
  {
    /* Is it the company we are looking for ? */
    if (BLE_COMPANY_IDS[i].id == comp_id)
    {
      /* Return the corresponding ble_company_id_t. */
      return &BLE_COMPANY_IDS[i];
    }
    
    /* Go to next record. */
    i++;
  }

  /* Not found. */
  return NULL;
}

int ble_adv_extract_name(uint8_t *p_adv_data, int adv_length, char *psz_device_name, int dest_length)
{
  int i=0, name_length=0;
  int8_t record_size, record_type;

  /* Check advertising PDU length, must be at least 2. */
  if (adv_length >= 2)
  {
    while (i < adv_length)
    {
      record_size = p_adv_data[i];
      if ((i + record_size + 1) < adv_length)
      {
        /* Get advertising record type. */
        record_type = p_adv_data[i+1];
        if (record_type == 0x08)
        {
          /* Complete name, we are done. */
          if (dest_length >= (record_size - 2))
          {
            /* Copy name to destination buffer. */
            strncpy(psz_device_name, (char *)&p_adv_data[i+2], record_size - 1);
            psz_device_name[record_size - 1] = '\0';
            name_length = record_size - 1;
          }
        }
        else if (record_type == 0x09)
        {
          /* Complete name, we are done. */
          if (dest_length >= (record_size - 2))
          {
            /* Copy name to destination buffer. */
            strncpy(psz_device_name, (char *)&p_adv_data[i+2], record_size - 1);
            psz_device_name[record_size - 1] = '\0';
            return record_size;
          }
        }
        else
        {
          /* Skip record if not short name or complete name. */
          i += record_size + 1;
        }
      }
      else
        return -1;
    }

    /* Return name_length. */
    return name_length;
  }
  else
    return -1;
}

ble_device_type_t ble_adv_classify(uint8_t *p_adv_data, int adv_length)
{
  int i, j, k;
  uint8_t record_size, record_type;
  int valid_records, nb_records;

  if (adv_length < 2)
    return UNKNOWN;

  /* Iterate over our DB. */
  j = 0;
  while (BLE_DEVICES_FP[j].patterns != NULL)
  {
    /* Count number of records. */
    nb_records = 0;
    while (BLE_DEVICES_FP[j].patterns[nb_records++].pattern != NULL);
    nb_records--;

    valid_records = 0;
    i = 0;
    while (i < adv_length)
    {
      record_size = p_adv_data[i];
      if ((i + record_size + 1) <= adv_length)
      {
        /* Get advertising record type. */
        record_type = p_adv_data[i+1];

        /* Check if the record is in our known patterns. */
        k = 0;
        while (BLE_DEVICES_FP[j].patterns[k].pattern != NULL)
        {
          if ((BLE_DEVICES_FP[j].patterns[k].record_type == record_type) && ((record_size-1) >= BLE_DEVICES_FP[j].patterns[k].pattern_size))
          {
            if (!memcmp(&p_adv_data[i+2], BLE_DEVICES_FP[j].patterns[k].pattern, BLE_DEVICES_FP[j].patterns[k].pattern_size))
            {
              /* Count this AD record as a valid one. */
              valid_records++;

              /* Exit while loop. */
              break;
            }
          }

          /* Go to next AD record pattern. */
          k++;
        }

        /* Skip record. */
        i += record_size + 1;
      }
      else
        break;
    }

    /* Do we have an exact match ? */
    if (nb_records == valid_records)
    {
      ESP_LOGI(TAG, "fingerprint found, device_type: %d", BLE_DEVICES_FP[j].device_type);

      /* Yes, return device type. */
      return BLE_DEVICES_FP[j].device_type;
    }

    /* Go to next fingerprint. */
    j++;
  }

  /* Default: unknown device. */
  return UNKNOWN;
}


int IRAM_ATTR on_ble_gap_event(struct ble_gap_event *event, void *arg)
{
  int i,j;
  bool b_device_known = false;

  switch(event->type)
  {
    case BLE_GAP_EVENT_CONNECT:
      {
        /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0) {

            /* Save current connection handle. */
            g_ble_ctrl.conn_handle = event->connect.conn_handle;
            g_ble_ctrl.b_connected = true;
            printf("conn handle: %d\n", g_ble_ctrl.conn_handle);

            uint8_t pdu[] = {0x0C, 0x08, 0x00, 0x00, 0x00, 0x00};

            /* Erase version info. */
            g_ble_ctrl.version_ble = 0;
            g_ble_ctrl.version_compid = 0;
            g_ble_ctrl.version_soft = 0;

            /* Send VERSION_IND PDU. */
            send_raw_data_pdu(
              g_ble_ctrl.conn_handle,
              0x03,
              pdu, // VERSION_IND PDU
              6,
              true
            );

            /* Connection successfully established. */
            ESP_LOGI(TAG, "Connection established, VERSION_IND sent !");
        } else {
            /* Connection attempt failed; resume scanning. */
            ESP_LOGE(TAG, "Error: Connection failed; status=%d\n",
                        event->connect.status);

            /* Post a BLE_DEVICE_FOUND event. */
            esp_event_post_to(
              g_ble_ctrl.evt_loop_handle,
              BLE_CTRL_EVENT,
              BLE_DEVICE_DISCONNECTED,
              NULL,
              0,
              portMAX_DELAY
            );
        }
      }
      break;

    case BLE_GAP_EVENT_DISC:
      {
        switch(event->disc.event_type)
        {
          /* Received an advertising report. */
          case BLE_HCI_ADV_RPT_EVTYPE_ADV_IND:
            {
              //ESP_LOGI(TAG, "received an advertising report");
              
              if (ble_enter_critical_section() == ESP_OK)
              {
                /* Check if we got a new device. */
                for (i=0; i<g_devices_nb; i++)
                {
                  if (!memcmp(&g_devices[i].address.val, &event->disc.addr.val, 6))
                  {
                    /* Parse advertising data to extract name. */
                    ble_adv_extract_name((uint8_t *)event->disc.data, event->disc.length_data, g_devices[i].psz_name, BLE_DEVICE_NAME_MAXSIZE);

                    /* Device has been found. */
                    b_device_known = true;

                    /* Update freshness. */
                    g_devices[i].freshness = BLE_FRESHNESS_DEFAULT;
                  }
                  else
                  {
                    if (g_devices[i].freshness == 0)
                    {
                      /* Remove device from list. */
                      if (i == (g_devices_nb - 1))
                      {
                        g_devices_nb--;
                      }
                      else
                      {
                        memcpy((void *)&g_devices[i], (void *)&g_devices[i+1], sizeof(ble_device_t) * (g_devices_nb - i - 1));
                        g_devices_nb--;

                        /* Process again the same item as we replaced it with the next item. */
                        i--;
                      }

                    }
                    else
                    {
                      g_devices[i].freshness--;
                    }
                  }
                }

                /* Don't notify already known devices. */
                if (b_device_known)
                {
                  //printf("Device known, exiting.\r\n");
                  ble_leave_critical_section();
                  return 0;
                }

                /* Add device to list, if a slot remains. */
                if (g_devices_nb < BLESCAN_DEVICES_MAX)
                {
                  /* Debug: show AD records for device. */
                  ESP_LOGI(TAG, "Device found: %02x:%02x:%02x:%02x:%02x:%02x",
                  event->disc.addr.val[5],
                  event->disc.addr.val[4],
                  event->disc.addr.val[3],
                  event->disc.addr.val[2],
                  event->disc.addr.val[1],
                  event->disc.addr.val[0]
                  );
                  
                  printf("AD records: ");
                  for (j=0; j<event->disc.length_data; j++)
                  {
                    printf("%02x", event->disc.data[j]);
                  }
                  printf("\r\n");

                  /* Classify device based on its AD records. */
                  g_devices[i].device_type = ble_adv_classify((uint8_t *)event->disc.data, event->disc.length_data);

                  /* Copy bluetooth address. */
                  memcpy(&g_devices[g_devices_nb].address, &event->disc.addr, sizeof(ble_addr_t));

                  /* Save RSSI. */
                  g_devices[g_devices_nb].i8_rssi = event->disc.rssi;
                  
                  /* No device name by default. */
                  g_devices[i].psz_name[0] = '\0';

                  /* Set freshness. */
                  g_devices[i].freshness = BLE_FRESHNESS_DEFAULT;

                  /* Parse advertising data to extract name. */
                  if (ble_adv_extract_name(
                    (uint8_t *)event->disc.data,
                    event->disc.length_data,
                    g_devices[g_devices_nb++].psz_name,
                    BLE_DEVICE_NAME_MAXSIZE) > 0)
                  {
                    //printf("Device found: %s\n", g_devices[i].psz_name);
                  }
                }

                /* Post a BLE_DEVICE_FOUND event. */
                esp_event_post_to(
                  g_ble_ctrl.evt_loop_handle,
                  BLE_CTRL_EVENT,
                  BLE_DEVICE_FOUND,
                  &g_devices[g_devices_nb-1],
                  sizeof(ble_device_t *),
                  portMAX_DELAY
                );

                ble_leave_critical_section();
              }
              else
              {
                ESP_LOGI(TAG, "ble_enter_critical_section() failed");
              }
            }
            break;

          case BLE_HCI_ADV_RPT_EVTYPE_SCAN_RSP:
            {
              //ESP_LOGI(TAG, "received a scan response");
              if (ble_enter_critical_section() == ESP_OK)
              {
                /* Check if we got a new device. */
                for (i=0; i<g_devices_nb; i++)
                {
                  if (!memcmp(&g_devices[i].address.val, &event->disc.addr.val, 6))
                  {
                    /* Parse advertising data to extract name. */
                    if (ble_adv_extract_name((uint8_t *)event->disc.data, event->disc.length_data, g_devices[i].psz_name, BLE_DEVICE_NAME_MAXSIZE) > 0)
                    {
                      /* Post a BLE_DEVICE_FOUND event. */
                      esp_event_post_to(
                        g_ble_ctrl.evt_loop_handle,
                        BLE_CTRL_EVENT,
                        BLE_DEVICE_FOUND,
                        &g_devices[i],
                        sizeof(ble_device_t *),
                        portMAX_DELAY
                      );
                    }

                    ble_leave_critical_section();
                    return 0;
                  }
                }
                
                ble_leave_critical_section();
              }
              else
              {
                ESP_LOGI(TAG, "ble_enter_critical_section() failed");
              }
            }
            break;

          default:
            break;
        }
      }
      break;

    case BLE_GAP_EVENT_DISCONNECT:
      {
        printf("DISCONNECTED\n");
        /* We have been disconnected. */
        g_ble_ctrl.b_connected = false;

        /* Post a BLE_DEVICE_FOUND event. */
        esp_event_post_to(
          g_ble_ctrl.evt_loop_handle,
          BLE_CTRL_EVENT,
          BLE_DEVICE_DISCONNECTED,
          NULL,
          0,
          portMAX_DELAY
        );

      }
      break;

    default:
        break;
  }
  return 0;
}

static void
ble_on_reset(int reason)
{
    printf("Resetting state; reason=%d\n", reason);
}

static void
ble_on_sync(void)
{
  g_ble_ctrl.mode = BLE_IDLE;
}


/**
 * ble_ctrl_event_handler_register()
 * @brief: registers an event handler for our custom event loop.
 * @param event_id: an integer representing the event that must match for this handler
 * @param event_handler: a pointer to a `esp_event_handler_t` callback function
 * @param event_handler_arg: a pointer to the event data that will be passed to this function.
 * @return: ESP_OK on success, other error values otherwise.
 **/

esp_err_t ble_ctrl_event_handler_register(
  int32_t event_id,
  esp_event_handler_t event_handler,
  void* event_handler_arg
)
{
  return esp_event_handler_instance_register_with(
    g_ble_ctrl.evt_loop_handle,
    BLE_CTRL_EVENT,
    event_id,
    event_handler,
    event_handler_arg,
    NULL
  );
}

int on_llcp_pdu_handler(uint16_t header, uint8_t *p_pdu, int length)
{
  uint8_t ble_version;
  uint16_t *comp_id;
  uint16_t *fw_version;
  ble_device_version_t *p_dev_version = NULL;

  /* Block VERSION_IND control PDU. */
  if (p_pdu[0] == 0x0C)
  {
    /* Display information about target version. */
    ble_version = p_pdu[1];
    comp_id = (uint16_t *)&p_pdu[2];
    fw_version = (uint16_t *)&p_pdu[4];
    esp_rom_printf("Target version: BLE(%d), Company(%04x), Fw(%04x)\r\n", ble_version, *comp_id, *fw_version);

    /* Keep track of version info. */
    g_ble_ctrl.version_ble = ble_version;
    g_ble_ctrl.version_compid = (uint16_t)(*comp_id);
    g_ble_ctrl.version_soft = (uint16_t)(*fw_version);

      #if 0
      ble_enter_critical_section();
      
      /* Post a BLE_DEVICE_VERSION event. */
      esp_event_post_to(
        g_ble_ctrl.evt_loop_handle,
        BLE_CTRL_EVENT,
        BLE_DEVICE_VERSION,
        p_dev_version,
        sizeof(ble_device_version_t),
        portMAX_DELAY
      );
      
      ble_leave_critical_section();
      #endif
    return HOOK_BLOCK;
  }
  else
    return HOOK_FORWARD;
}


/**
 * @brief Initialize our BLE controller.
 **/

void ble_ctrl_init(void)
{
    int rc;

    /* Initialize NVS. */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize NimBLE. */
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();

    /* Install our hooks. */
    ble_hack_install_hooks();
    ble_hack_rx_control_pdu_handler(on_llcp_pdu_handler);

    /* Configure the host. */
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    /* Initialize our event loop. */
    g_ble_ctrl.evt_loop_args.queue_size = 5;
    g_ble_ctrl.evt_loop_args.task_name = "ble_ctrl_task";
    g_ble_ctrl.evt_loop_args.task_priority = uxTaskPriorityGet(NULL);
    g_ble_ctrl.evt_loop_args.task_stack_size = 2048;
    g_ble_ctrl.evt_loop_args.task_core_id = tskNO_AFFINITY;
    g_ble_ctrl.evt_loop_initialized = false;

    /* Create our custom loop. */
    if (esp_event_loop_create(&g_ble_ctrl.evt_loop_args, &g_ble_ctrl.evt_loop_handle) == ESP_OK)
    {
      ESP_LOGI(TAG, "event loop successfully created for ble controller.");
    }
    else
      ESP_LOGE(TAG, "cannot create event loop for ble controller.");

    /* Create our mutex. */
    g_ble_ctrl.mutex = xSemaphoreCreateMutex();

    /* Initialize our controller. */
    g_ble_ctrl.b_connected = false;
    g_ble_ctrl.conn_handle = 0;
    g_ble_ctrl.mode = BLE_OFF;

    nimble_port_freertos_init(ble_host_task);
}


/**
 * @brief Start GAP discovery procedure.
 * @return 0 on success, -1 otherwise.
 **/

int ble_scan_start(void)
{
  int rc;

  struct ble_gap_disc_params scan_params = {
    .itvl = 0, .window = 0, .filter_policy = 0,
    .limited = 0, .passive = 0, .filter_duplicates = 0
  };

  /* Make sure we have proper identity address set (public preferred) */
  rc = ble_hs_util_ensure_addr(0);
  assert(rc == 0);

  /* Begin scanning for a peripheral to connect to. */
  if (ble_gap_disc(0, BLE_HS_FOREVER, &scan_params, on_ble_gap_event, NULL) != 0)
  {
    ESP_LOGI(TAG, "An unexpected error occured");
    return -1;
  };

  ESP_LOGI(TAG, "BLE scanning in progress...");
  g_ble_ctrl.mode = BLE_SCANNER;
  return 0;
}


/**
 * @brief Stop active GAP discovery procedure (if any)
 * @return 0 on success, !=0 on error.
 */

int ble_scan_stop(void)
{
  int result = 0;

  if (ble_gap_disc_active())
  {
    ESP_LOGI(TAG, "BLE scanning stopped.");
    result = ble_gap_disc_cancel();
    g_ble_ctrl.mode = BLE_OFF;
  }

  return result;
}


/**
 * @brief Initiate a connection to a given BLE device
 * @param device: pointer to a struct `ble_device_t` that represents the target device
 * @return int: 0 on success, <0 otherwise.
 **/

int ble_connect(ble_device_t *device)
{
  uint8_t own_addr_type;
  int rc;

  if (!ble_scan_stop())
  {
    /* Figure out address to use for connect (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "error determining address type; rc=%d\n", rc);
        return -1;
    }

    /* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for
      * timeout.
      */
    rc = ble_gap_connect(own_addr_type, &device->address, 5000, NULL,
                          on_ble_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error: Failed to connect to device; addr_type=%d "
                            "rc=%d",
                    device->address.type, rc);
        return -1;
    }

    g_ble_ctrl.mode = BLE_FINGERPRINT;
    return 0;
  }
  else
  {
    ESP_LOGE(TAG, "Cannot stop scan!");
    return -1;
  }
  
}


/**
 * @brief Terminate the current connection (if any) and restart scanning.
 **/

int ble_disconnect(void)
{
  int res = -1;

  if (g_ble_ctrl.b_connected)
  {
    //_llc_llcp_terminate_ind_pdu_send(g_ble_ctrl.conn_handle, 0x00);
    res = ble_gap_terminate(g_ble_ctrl.conn_handle, 0x00);
  }
  //_llc_llcp_terminate_ind_pdu_send(g_ble_ctrl.conn_handle, 0x00);

  /* Switch back into scan mode. */
  ble_scan_start();

  return res;
}

/**
 * @brief determine if a BLE scan is in progress.
 * @return 1 if scan is active, 0 otherwise.
 **/

int ble_scan_active(void)
{
  return ble_gap_disc_active();
}


/**
 * @brief Get the current number of detected devices
 * @return int: number of devices in our list.
 **/

int ble_get_devices_nb(void)
{
  int nb_devices = 0;

  nb_devices = g_devices_nb;
  
  return nb_devices;
}


/**
 * @brief: Enter BLE critical section (mutex)
 **/

esp_err_t ble_enter_critical_section(void)
{
  if (xSemaphoreTake(g_ble_ctrl.mutex, 100/portTICK_PERIOD_MS) == pdTRUE)
  {
    return ESP_OK;
  }
  else
    return ESP_FAIL;
}


/**
 * @brief: Exit BLE critical section (mutex)
 **/

void ble_leave_critical_section(void)
{
  xSemaphoreGive(g_ble_ctrl.mutex);
}


/**
 * @brief: Return information about a detected device
 * @param index: index of the required device in our list
 * @return Pointer to a `ble_device_t` structure containing device information.
 **/

ble_device_t *ble_get_device(int index)
{
  ble_device_t *p_device = NULL;
  
  if (index < g_devices_nb)
  {
    p_device = &g_devices[index];
    return p_device;
  }
  else
  {
    return NULL;
  }
}



uint8_t ble_device_get_version(void)
{
  return g_ble_ctrl.version_ble;
}

uint16_t ble_device_get_compid(void)
{
  return g_ble_ctrl.version_compid;
}

uint16_t ble_device_get_soft(void)
{
  return g_ble_ctrl.version_soft;
}