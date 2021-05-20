#include "tile-settings.h"

static tile_t settings_one_tile, settings_two_tile, settings_three_tile;
static widget_value_select_t hours_select, mins_select, lbl_clock;
static widget_value_select_t days_select, months_select, years_select;
static rtc_datetime_t datetime;
static widget_label_t lbl_one_title, lbl_two_title, lbl_three_title;
static widget_label_t lbl_month, lbl_year;
static widget_button_t btn_orientation;
static widget_button_t btn_save_clock, btn_save_date;
static widget_timeset_t timeset;

/**
 * Custom widget: value selection.
 **/

int _constrain(int min, int max, int value)
{
  if (value < min)
    return max;
  else if (value > max)
    return min;
  return value;
}

void widget_value_select_update_value(widget_value_select_t *p_value_select)
{
  /* Set label text. */
  snprintf(p_value_select->psz_label_text, 3, "%02d", p_value_select->current_value);
  widget_label_set_text(
    &p_value_select->label,
    p_value_select->psz_label_text
  );
}

int widget_value_select_eventhandler(widget_t *p_widget, widget_event_t p_event, int x, int y, int velocity)
{
  widget_value_select_t *p_value_select = (widget_value_select_t *)p_widget;

  switch(p_event)
  {
    case WE_SWIPE_UP:
      p_value_select->current_value = _constrain(p_value_select->min_value, p_value_select->max_value, p_value_select->current_value + 1);
      widget_value_select_update_value(p_value_select);
      return WE_PROCESSED;
      
    case WE_SWIPE_DOWN:
      p_value_select->current_value = _constrain(p_value_select->min_value, p_value_select->max_value, p_value_select->current_value - 1);
      widget_value_select_update_value(p_value_select);
      return WE_PROCESSED;

    default:
      return WE_ERROR;
  }
}

void widget_value_select_init(
  widget_value_select_t *p_value_select,
  tile_t *p_tile,
  int x,
  int y,
  int width,
  int height,
  int min_value,
  int max_value,
  int current_value
)
{
  /* Initialize our label. */
  widget_label_init(&p_value_select->label, p_tile, x, y, width, height, "");
  widget_set_bg_color(&p_value_select->label.widget, RGB(0x3,0x3,0x3));
  widget_set_front_color(&p_value_select->label.widget, RGB(0x5,0x9,0xf));
  widget_set_eventhandler(&p_value_select->label.widget, widget_value_select_eventhandler);

  /* Set min value. */
  p_value_select->min_value = _constrain(0, 99, min_value);

  /* Set max value. */
  p_value_select->max_value = _constrain(p_value_select->min_value, 99, max_value);

  /* Set current value. */
  p_value_select->current_value = _constrain(p_value_select->min_value, p_value_select->max_value, current_value);

  /* Set label text. */
  widget_value_select_update_value(p_value_select);
}

int widget_value_select_get_value(widget_value_select_t *p_value_select)
{
  return p_value_select->current_value;
}

void widget_value_select_set_value(widget_value_select_t *p_value_select, int value)
{
  p_value_select->current_value = value;
  widget_value_select_update_value(p_value_select);
}

void clock_save_onclick(widget_t *p_widget)
{
  int hours,minutes;

  /* Read hours and minutes. */
  hours = widget_value_select_get_value(&hours_select);
  minutes = widget_value_select_get_value(&mins_select);

  /* Set datetime. */
  twatch_rtc_get_date_time(&datetime);
  datetime.hour = hours;
  datetime.minute = minutes;
  datetime.second = 0;
  twatch_rtc_set_date_time(&datetime);
}

void date_save_onclick(widget_t *p_widget)
{
  int days,months,years;

  /* Read days and months. */
  days = widget_value_select_get_value(&days_select);
  months = widget_value_select_get_value(&months_select);
  years = widget_value_select_get_value(&years_select);

  /* Set datetime. */
  twatch_rtc_get_date_time(&datetime);
  datetime.day = days;
  datetime.month = months;
  datetime.year = years;
  twatch_rtc_set_date_time(&datetime);
}

void settings_invert_onclick(widget_t *p_widget)
{
  if (twatch_screen_is_inverted())
  {
    /* Disable inverted screen. */
    twatch_screen_set_inverted(false);
    twatch_touch_set_inverted(false);
  }
  else
  {
    /* Enable inverted screen. */
    twatch_screen_set_inverted(true);
    twatch_touch_set_inverted(true);
  }
}

int settings_one_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Get date and time. */
        twatch_rtc_get_date_time(&datetime);

        //widget_value_select_set_value(&hours_select, datetime.hour);
        //widget_value_select_set_value(&mins_select, datetime.minute);
      }
      break;
    
    default:
      break;
  }

  /* Success. */
  return TE_PROCESSED;
}

int settings_two_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Get date and time. */
        twatch_rtc_get_date_time(&datetime);
        uint8_t year = datetime.year%100;

        widget_value_select_set_value(&days_select, datetime.day);
        widget_value_select_set_value(&months_select, datetime.month);
        widget_value_select_set_value(&years_select, year);
      }
      break;
  
    default:
      break;
  }

  /* Success. */
  return TE_PROCESSED;
}

tile_t *tile_settings_one_init(void)
{
  /* Get date and time. */
  twatch_rtc_get_date_time(&datetime);

  /* Initialize our tile. */
  tile_init(&settings_one_tile, NULL);
  tile_set_event_handler(&settings_one_tile, settings_one_tile_event_handler);

  /* Initialize our title label. */
  widget_label_init(&lbl_one_title, &settings_one_tile, 10, 5, 230, 45, "Settings 1/3");

  timeset_init(&timeset, &settings_one_tile, 0, 120, 0, 0);

#if 0
  /* Initialize our hours selection widget. */
  widget_value_select_init(&hours_select, &settings_one_tile, 65, (240-50)/2, 40, 40, 0, 23, datetime.hour);
  widget_label_init(&lbl_clock, &settings_one_tile, 115, (240-50)/2, 20, 45, ":");

  /* Initialize our hours selection widget. */
  widget_value_select_init(&mins_select, &settings_one_tile, 135, (240-50)/2, 40, 40, 0, 59, datetime.minute);
#endif

  /* Initialize our buttons. */
  //widget_button_init(&btn_save_clock, &settings_one_tile, 15, 190, 210, 45, "Save clock");
  //widget_button_set_handler(&btn_save_clock, clock_save_onclick);

  /* Return our tile. */
  return &settings_one_tile;
}

tile_t *tile_settings_two_init(void)
{
  /* Get date and time. */
  twatch_rtc_get_date_time(&datetime);
  uint8_t year = datetime.year%100;

  /* Initialize our tile. */
  tile_init(&settings_two_tile, NULL);
  tile_set_event_handler(&settings_two_tile, settings_two_tile_event_handler);

  /* Initialize our title label. */
  widget_label_init(&lbl_two_title, &settings_two_tile, 10, 5, 230, 45, "Settings 2/3");

    /* Initialize our day selection widget. */
  widget_value_select_init(&days_select, &settings_two_tile, 10, 90, 40, 40, 1, 31, datetime.day);

  /* Initialize our month selection widget. */
  widget_label_init(&lbl_month, &settings_two_tile, 55, 90, 20, 45, "/");
  widget_value_select_init(&months_select, &settings_two_tile, 80, 90, 40, 40, 0, 12, datetime.month);

  /* Initialize our year selection widget. */
  widget_label_init(&lbl_year, &settings_two_tile, 125, 90, 80, 45, "/ 20");
  widget_value_select_init(&years_select, &settings_two_tile, 190, 90, 40, 40, 21, 99, year);
  
  /* Initialize our buttons. */
  widget_button_init(&btn_save_date, &settings_two_tile, 15, 190, 210, 45, "Save date");
  widget_button_set_handler(&btn_save_date, date_save_onclick);

  /* Return our tile. */
  return &settings_two_tile;
}

tile_t *tile_settings_three_init(void)
{
  /* Initialize our tile. */
  tile_init(&settings_three_tile, NULL);

  /* Initialize our title label. */
  widget_label_init(&lbl_three_title, &settings_three_tile, 10, 5, 230, 45, "Settings 3/3");
  
  widget_button_init(&btn_orientation, &settings_three_tile, 15, (240-45)/2, 210, 45, "Rotate screen");
  widget_button_set_handler(&btn_orientation, settings_invert_onclick);

  /* Return our tile. */
  return &settings_three_tile;
}
