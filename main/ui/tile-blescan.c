#include "tile-blescan.h"

#define BLESCAN_LISTITEM_HEIGHT 45

tile_t blescan_tile;

blescan_widget_listitem_t g_devices_names[10];
char names[10][BLE_DEVICE_NAME_MAXSIZE];
widget_label_t lbl_scanner;
widget_listbox_t lb_devices;
volatile int g_devices_nb;
FEventHandler pfn_lb_event_handler;
modal_t *p_bleinfo_modal;

#undef TAG
#define TAG "[tile::ble::scanner]"

/*******************************************
 * BleDevice list item widget
 ******************************************/

/**
 * blescan_widget_listitem_event_handler()
 * 
 * @brief: BLE scan widget listitem event handler.
 * @param event: UI event.
 * @param x: X screen coordinate
 * @param y: Y screen coordinate
 * @param velocity: swipe speed (not used).
 * @return: WE_PROCESSED if event has been processed, WE_ERROR otherwise.
 **/

int blescan_widget_listitem_event_handler(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  bool b_processed = false;
  blescan_widget_listitem_t *p_listitem = (blescan_widget_listitem_t *)p_widget->p_user_data;

  if (p_listitem != NULL)
  {
    switch (event)
    {

      case LB_ITEM_SELECTED:
        {
          /* Mark list item as selected. */
          p_listitem->b_selected = true;
          b_processed = true;
        }
        break;

      case LB_ITEM_DESELECTED:
        {
          p_listitem->b_selected = false;
          b_processed = true;
        }
        break;

      default:
        break;
    }

    /* Event processed (or not) */
    return b_processed;
  }

  /* Event not processed. */
  return WE_ERROR;
}


/**
 * wscan_widget_listitem_drawfunc()
 * 
 * @brief: Wifiscan listitem widget drawing function.
 * @param p_widget: pointer to a `widget_t` structure.
 **/

int blescan_widget_listitem_drawfunc(widget_t *p_widget)
{
  char bdaddr[18];
  char channel[4];

  blescan_widget_listitem_t *p_listitem = (blescan_widget_listitem_t *)p_widget->p_user_data;

  if (p_listitem != NULL)
  {
    if (p_listitem->p_device != NULL)
    {
      //ble_enter_critical_section();

      /* Draw background. */
      widget_fill_region(
        p_widget,
        1,
        1,
        p_widget->box.width - 2,
        p_widget->box.height - 2,
        (p_listitem->b_selected)?p_widget->style.front:p_widget->style.background
      );

      /* Draw BD address. */
      snprintf(
        bdaddr,
        18,
        "%02X:%02X:%02X:%02X:%02X:%02X",
        p_listitem->p_device->address.val[5],
        p_listitem->p_device->address.val[4],
        p_listitem->p_device->address.val[3],
        p_listitem->p_device->address.val[2],
        p_listitem->p_device->address.val[1],
        p_listitem->p_device->address.val[0]
      );
      widget_draw_text(
        p_widget,
        5,
        5,
        (char *)bdaddr,
        (p_listitem->b_selected)?p_widget->style.background:p_widget->style.front
      );

      //ble_leave_critical_section();
    }
  }

  /* Success. */
  return TE_PROCESSED;
}



void blescan_widget_listitem_init(blescan_widget_listitem_t *p_listitem, ble_device_t *p_device)
{
  /* Initialize our widget. */
  widget_init(&p_listitem->widget, NULL, 0, 0, 0, BLESCAN_LISTITEM_HEIGHT);

  /* Copy name and BD address. */
  p_listitem->p_device = p_device;

  /* Set userdata. */
  widget_set_userdata(&p_listitem->widget, (void *)p_listitem);

  /* Set drawing function. */
  widget_set_drawfunc(&p_listitem->widget, blescan_widget_listitem_drawfunc);

  /* Set default event handler. */
  widget_set_eventhandler(&p_listitem->widget, blescan_widget_listitem_event_handler);
}

void blescan_widget_listitem_update(blescan_widget_listitem_t *p_listitem, ble_device_t *p_device)
{
  /* Update BLE device. */
  p_listitem->p_device = p_device;
}





int scanner_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Enable BLE scanner. */
        ble_scan_start();

        /* Disable eco mode. */
        disable_ecomode();
      }
      break;

    case TE_EXIT:
      {
        /* Stop scanner. */
        ble_scan_stop();

        /* Re-enable eco mode. */
        enable_ecomode();
      }
      break;

    default:
      break;
  }

  /* Success. */
  return TE_PROCESSED;
}

/**
 * blescan_widget_list_event_handler()
 * 
 * @brief: Wifi scan widget listitem event handler.
 * @param event: UI event.
 * @param x: X screen coordinate
 * @param y: Y screen coordinate
 * @param velocity: swipe speed (not used).
 * @return: WE_PROCESSED if event has been processed, WE_ERROR otherwise.
 **/

int blescan_widget_list_event_handler(widget_t *p_widget, widget_event_t event, int x, int  y, int velocity)
{
  int index, i;
  ble_device_t *device;
  bool b_processed = false;
  widget_listbox_t *p_listbox = (widget_listbox_t *)p_widget;
  blescan_widget_listitem_t *p_listitem;

  ble_enter_critical_section();

  switch (event)
  {

    case LB_ITEM_SELECTED:
      {
        /* Retrieve selected item. */
        p_listitem = (blescan_widget_listitem_t *)p_listbox->p_selected_item;
        if (p_listitem != NULL)
        {
          ESP_LOGI(TAG, "Item selected !");
          
          /* Clear modal. */
          modal_bleinfo_clear();
          modal_bleinfo_wait();

          /* Connect to the selected device. */
          ble_connect(p_listitem->p_device);

          /* Set modal title. */
          modal_bleinfo_set_title(p_listitem->p_device);

          /* Show modal. */
          ui_set_modal(p_bleinfo_modal);
        }

        b_processed = true;
      }
      break;

    default:
      break;
  }

  /* Event processed (or not) */
  ble_leave_critical_section();
  return b_processed?WE_PROCESSED:pfn_lb_event_handler(p_widget, event, x, y, velocity);
}

void on_device_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  int i;
  ble_device_t *peer;
  char psz_label[18];
  ble_device_version_t *p_version = NULL;

  switch (event_id)
  {
    case BLE_DEVICE_VERSION:
      {
        p_version = (ble_device_version_t *)(event_data);
        modal_bleinfo_update(p_version->ble, p_version->company, p_version->software);
      }
      break;

    case BLE_DEVICE_FOUND:
      {
        ble_enter_critical_section();
        ui_enter_critical_section();
        
        /* Empty list. */
        for (i=0; i<10; i++)
        {
          widget_listbox_remove(&lb_devices, (widget_t *)&g_devices_names[i]);
        }

        g_devices_nb = ble_get_devices_nb();
        
        /* Add device. */
        for (i=0; i<g_devices_nb; i++)
        {
          peer = ble_get_device(i);
          blescan_widget_listitem_update(&g_devices_names[i], peer);
          widget_listbox_add(&lb_devices, (widget_t *)&g_devices_names[i]);
        }
        
        ui_leave_critical_section();
        ble_leave_critical_section();
      }
      break;

    case BLE_DEVICE_DISCONNECTED:
      {
        /* Hide modal. */
        ui_unset_modal();
      }
      break;

    default:
      break;
  }
}

void on_device_found(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  int i;
  ble_device_t *peer;
  char psz_label[18];

  ble_enter_critical_section();
  ui_enter_critical_section();

  /* Empty list. */
  for (i=0; i<10; i++)
  {
    widget_listbox_remove(&lb_devices, (widget_t *)&g_devices_names[i]);
  }

  g_devices_nb = ble_get_devices_nb();
  
  /* Add device. */
  for (i=0; i<g_devices_nb; i++)
  {
    peer = ble_get_device(i);
    blescan_widget_listitem_update(&g_devices_names[i], peer);
    widget_listbox_add(&lb_devices, (widget_t *)&g_devices_names[i]);
  }
  
  ui_leave_critical_section();
  ble_leave_critical_section();
}

tile_t *tile_blescan_init(void)
{
  int i;

  /* Initialize our modal box. */
  p_bleinfo_modal = modal_bleinfo_init();

  /* Initialize our tile. */
  tile_init(&blescan_tile, NULL);
  tile_set_event_handler(&blescan_tile, scanner_tile_event_handler);

  /* Initialize our title label */
  widget_label_init(&lbl_scanner, &blescan_tile, 3, 3, 237, 36, "BLE Devices");

  /* Initialize our listbox. */
  widget_listbox_init(&lb_devices, &blescan_tile, 3, 45, 234, 190);

  /* Set default event handler. */
  pfn_lb_event_handler = widget_set_eventhandler(&lb_devices.widget, blescan_widget_list_event_handler);
  
  /* Install our BLE event listener. */
  ble_ctrl_event_handler_register(
    BLE_DEVICE_FOUND,
    on_device_event,
    NULL
  );

  ble_ctrl_event_handler_register(
    BLE_DEVICE_VERSION,
    on_device_event,
    NULL
  );

  ble_ctrl_event_handler_register(
    BLE_DEVICE_DISCONNECTED,
    on_device_event,
    NULL
  );

  /* Initialize our labels. */
  for (i=0; i<10; i++)
  {
    blescan_widget_listitem_init(&g_devices_names[i], NULL);
  }

  g_devices_nb = 0;

  /* Return our tile. */
  return &blescan_tile;
}