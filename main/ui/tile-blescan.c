#include "tile-blescan.h"

#include "../img/ble_icons.h"
#include "../img/ble_icons_big.h"

#define BLESCAN_LISTITEM_HEIGHT 35
#define BLESCAN_LISTITEM_OFFY   5  
#undef TAG
#define TAG "[tile::ble::scanner]"

/*******************************************
 * Globals
 ******************************************/

tile_t blescan_tile;
blescan_widget_listitem_t g_devices_names[10];
char names[10][BLE_DEVICE_NAME_MAXSIZE];
widget_label_t ble_lbl_scanner;
widget_listbox_t lb_devices;
volatile int g_ble_devices_nb;
FEventHandler pfn_lb_event_handler;
modal_t *p_bleinfo_modal;
image_t *icons, *icons_big;


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

  blescan_widget_listitem_t *p_listitem = (blescan_widget_listitem_t *)p_widget->p_user_data;

  if (p_listitem != NULL)
  {
    if (p_listitem->p_device != NULL)
    {
      /* Draw background. */
      widget_fill_region(
        p_widget,
        1,
        1 + BLESCAN_LISTITEM_OFFY,
        p_widget->box.width - 2,
        p_widget->box.height - 2 - BLESCAN_LISTITEM_OFFY,
        p_widget->style.background
      );

      /* Draw BD address. */
      if (!p_listitem->p_device->psz_name[0])
      {
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
          31,
          5 + BLESCAN_LISTITEM_OFFY,
          (char *)bdaddr,
          p_widget->style.front
        );
      }
      else
      {
        widget_draw_text(
          p_widget,
          31,
          5 + BLESCAN_LISTITEM_OFFY,
          (char *)p_listitem->p_device->psz_name,
          p_widget->style.front
        );
      }

      /* Draw icon. */
      if (p_listitem->b_selected)
      {
        widget_bitblt(
          p_widget,
          icons_big,
          24*p_listitem->p_device->device_type,
          0,
          24,
          24,
          4,
          1 + BLESCAN_LISTITEM_OFFY
        );
      }
      else
      {
        widget_bitblt(
          p_widget,
          icons,
          16*p_listitem->p_device->device_type,
          0,
          16,
          16,
          9,
          5 + BLESCAN_LISTITEM_OFFY
        );
      }

      /* Draw a frame when selected. */
      if (p_listitem->b_selected)
      {
        widget_draw_line(
          p_widget,
          1,0,p_widget->box.width-2,0,RGB(0x1, 0x2, 0x3)
        );
        widget_draw_line(
          p_widget,
          1,1,p_widget->box.width-2,1,RGB(0x1, 0x2, 0x3)
        );

        widget_draw_line(
          p_widget,
          1,p_widget->box.height-1,p_widget->box.width-2,p_widget->box.height-1,RGB(0x1, 0x2, 0x3)
        );
        widget_draw_line(
          p_widget,
          1,p_widget->box.height-2,p_widget->box.width-2,p_widget->box.height-2,RGB(0x1, 0x2, 0x3)
        );


        widget_draw_line(
          p_widget,
          0,1,0,p_widget->box.height-2,RGB(0x1, 0x2, 0x3)
        );
        widget_draw_line(
          p_widget,
          1,1,1,p_widget->box.height-2,RGB(0x1, 0x2, 0x3)
        );

        widget_draw_line(
          p_widget,
          p_widget->box.width-1,1,p_widget->box.width-1,p_widget->box.height-2,RGB(0x1, 0x2, 0x3)
        );
        widget_draw_line(
          p_widget,
          p_widget->box.width-2,1,p_widget->box.width-2,p_widget->box.height-2,RGB(0x1, 0x2, 0x3)
        );

      }

    }
  }

  /* Success. */
  return TE_PROCESSED;
}


/**
 * @brief Initialize our BLE scanner list item
 * @param p_listitem: pointer to a `blescan_widget_listitem_t` structure
 * @param p_device: pointer to a `ble_device_t` structure, referencing the BLE device associated with this item
 **/

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


/**
 * @brief Update widget list item BLE device structure.
 * @param p_listitem: pointer to a `blescan_widget_listitem_t` structure
 * @param p_device: pointer to a `ble_device_t` structure that will be associated with this item
 **/

void blescan_widget_listitem_update(blescan_widget_listitem_t *p_listitem, ble_device_t *p_device)
{
  /* Update BLE device. */
  p_listitem->p_device = p_device;
}


/**
 * @brief BLE scanner tile event handler
 * @param p_tile: pointer to a `tile_t` structure referencing the current tile
 * @param event: event type
 * @param x: x-coordinate of the event (if required)
 * @param y: y-coordinate of the event (if required)
 * @param velocity: swipe velocity (only set for user interaction event)
 * @return TE_PROCESSED if event has been processed, TE_ERROR otherwise.
 **/

int ble_scanner_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
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


/**
 * @brief Event callback used to handle device events sent by the BLE controller
 * @param event_handler_arg: argument passed by the BLE controller
 * @param event_base: event category
 * @param event_id: event ID
 * @param event_data: pointer to data associated to the current event
 **/

void on_device_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  int i;
  ble_device_t *peer;
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

        g_ble_devices_nb = ble_get_devices_nb();
        
        /* Add device. */
        for (i=0; i<g_ble_devices_nb; i++)
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
        if (ble_device_get_version() == 0)
        {
          /* Hide modal. */
          ui_unset_modal();
        }
      }
      break;

    default:
      break;
  }
}


/**
 * @brief Initialize the BLE scanner tile
 * @return pointer to the initialized tile (`tile_t` structure)
 **/

tile_t *tile_blescan_init(void)
{
  int i;

  /* Load icons. */
  icons = load_image(ble_icons);
  icons_big = load_image(ble_icons_big);

  /* Initialize our modal box. */
  p_bleinfo_modal = modal_bleinfo_init();

  /* Initialize our tile. */
  tile_init(&blescan_tile, NULL);
  tile_set_event_handler(&blescan_tile, ble_scanner_tile_event_handler);

  /* Initialize our title label */
  widget_label_init(&ble_lbl_scanner, &blescan_tile, 3, 3, 237, 36, "BLE Devices");

  /* Initialize our listbox. */
  widget_listbox_init(&lb_devices, &blescan_tile, 3, 45, 234, 190);

  /* Set listbox border color. */
  widget_set_border_color(&lb_devices.widget, RGB(0x1, 0x2, 0x3));
  
  /* Set listbox's scrollbar border color. */
  widget_set_border_color(&lb_devices.scrollbar.widget, RGB(0x1, 0x2, 0x3));

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

  g_ble_devices_nb = 0;

  printf("blescan ok\n");

  /* Return our tile. */
  return &blescan_tile;
}