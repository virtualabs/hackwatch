#include "battery.h"
#include "hal/pmu.h"

/**
 * widget_battery_drawfunc()
 *
 * @brief: render function for battery widget
 * @param p_widget: pointer to a `widget_t` structure
 **/

void widget_battery_drawfunc(widget_t *p_widget)
{
  widget_batt_t *p_battery = (widget_batt_t *)p_widget->p_user_data;

  if (p_battery != NULL)
  {
    /* Read battery level each 50 iterations. */
    if (p_battery->delay < 50)
      p_battery->delay++;
    else
    {
      p_battery->percentage = twatch_pmu_get_battery_level();
      p_battery->delay = 0;
    }

    widget_draw_line(
      p_widget,
      1, 0, BATTERY_WIDTH - 1, 0, RGB(0xe, 0xe, 0xe)
    );
    widget_draw_line(
      p_widget,
      1, BATTERY_HEIGHT-1, BATTERY_WIDTH - 1, BATTERY_HEIGHT-1, RGB(0xe, 0xe, 0xe)
    );
    widget_draw_line(
      p_widget,
      0, 1, 0, BATTERY_HEIGHT - 2, RGB(0xe, 0xe, 0xe)
    );
    widget_draw_line(
      p_widget,
      BATTERY_WIDTH, 1, BATTERY_WIDTH, BATTERY_HEIGHT-2, RGB(0xe, 0xe, 0xe)
    );
    widget_draw_line(
      p_widget,
      BATTERY_WIDTH+1, BATTERY_HEIGHT/2 - 2, BATTERY_WIDTH+1, BATTERY_HEIGHT/2+2, RGB(0xe, 0xe, 0xe)
    );
    widget_draw_line(
      p_widget,
      BATTERY_WIDTH+2, BATTERY_HEIGHT/2 - 2, BATTERY_WIDTH+2, BATTERY_HEIGHT/2+2, RGB(0xe, 0xe, 0xe)
    );
    widget_fill_region(
      p_widget,
      1,
      1,
      ((BATTERY_WIDTH-1)*p_battery->percentage)/100,
      BATTERY_HEIGHT-2,
      twatch_pmu_is_usb_plugged(false)?RGB(0x0, 0x8, 0xc):RGB(0xe,0xe,0xe)
    );
  }
}


void widget_battery_init(widget_batt_t *p_battery, tile_t *p_tile, int x, int y)
{
  /* Initialize widget. */
  widget_init(&p_battery->widget, p_tile, x, y, BATTERY_WIDTH+3, BATTERY_HEIGHT+2);

  /* Read battery level. */
  p_battery->percentage = twatch_pmu_get_battery_level();

  /* Check if we are charging. */
  p_battery->b_usb_connected = twatch_pmu_is_usb_plugged(true);

  p_battery->delay = 0;

  /* Set user data. */
  widget_set_userdata(&p_battery->widget, (void *)p_battery);

  /* Define our drawing function. */
  widget_set_drawfunc(&p_battery->widget, (FDrawWidget)widget_battery_drawfunc);
}