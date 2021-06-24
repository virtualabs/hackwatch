#include "tile-settings.h"

static tile_t settings_one_tile, settings_two_tile, settings_three_tile;

/* Time saved confirm modal */
static modal_t confirm;
static widget_button_t confirm_btn;
static widget_label_t confirm_txt;

static widget_value_select_t hours_select, mins_select, lbl_clock;
static widget_value_select_t days_select, months_select, years_select;
static rtc_datetime_t datetime;
static widget_label_t lbl_one_title, lbl_two_title, lbl_three_title;
static widget_label_t lbl_month, lbl_year;
static widget_button_t btn_orientation;
static widget_button_t btn_save_clock, btn_save_date;
static widget_timeset_t timeset;
static widget_slider_t sld_backlight;

/**
 * _constrain()
 * 
 * @brief: Constrain `value` between `min` and `max`.
 * @param min: min value
 * @param max: max value
 * @param value: value to constrain
 * @return: constrained value.
 **/

int _constrain(int min, int max, int value)
{
  if (value < min)
    return max;
  else if (value > max)
    return min;
  return value;
}


/**
 * widget_value_select_update_value()
 * 
 * @brief: Update the value of a widget_value_select_t widget.
 * @param p_value_select: pointer to a `widget_value_select_t` structure
 **/

void widget_value_select_update_value(widget_value_select_t *p_value_select)
{
  /* Set label text. */
  snprintf(p_value_select->psz_label_text, 3, "%02d", p_value_select->current_value);
  widget_label_set_text(
    &p_value_select->label,
    p_value_select->psz_label_text
  );
}


/**
 * widget_value_select_eventhandler()
 * 
 * @brief: Select widget event handler.
 * @param p_widget: pointer to a `widget_t` structure
 * @param p_event: event
 * @param x: X-coordinate for the event
 * @param y: Y-coordinate for the event
 * @param velocity: swipe speed (in case of a swipe event)
 **/

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

/**
 * widget_value_select_init()
 * 
 * @brief: initialize a widget_value_select widget
 * @param p_tile: pointer to parent tile
 * @param x: X position of the widget
 * @param y: Y position of the widget
 * @param width: widget width
 * @param height: widget height
 * @param min_value: selection min value
 * @param max_value: selection max value
 * @param current_value: selection current value
 **/

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


/**
 * widget_value_select_get_value()
 * 
 * @brief: Retrieve the current value of a select value widget
 * @param p_value_select: pointer to a `widget_value_select_t` structure
 * @return current value
 **/

int widget_value_select_get_value(widget_value_select_t *p_value_select)
{
  return p_value_select->current_value;
}


/**
 * widget_value_select_set_value()
 * 
 * @brief: Set the current value of a select value widget
 * @param p_value_select: pointer to a `widget_value_select_t` structure
 * @param value: value to set
 **/

void widget_value_select_set_value(widget_value_select_t *p_value_select, int value)
{
  p_value_select->current_value = value;
  widget_value_select_update_value(p_value_select);
}


/**
 * clock_save_onclick()
 * 
 * @brief: time save button callback handler
 * @param p_widget: pointer to a `widget_t` structure
 **/

void clock_save_onclick(widget_t *p_widget)
{
  twatch_rtc_get_date_time(&datetime);
  timeset_get_time(&timeset, &datetime);
  twatch_rtc_set_date_time(&datetime);

  ui_set_modal(&confirm);
}

/**
 * clock_save_confirm()
 * 
 * @brief: time save confirm modal handler
 * @param p_widget: pointer to a `widget_t` structure
 **/

void clock_save_confirm(widget_t *p_widget)
{
  ui_unset_modal();
}


/**
 * date_save_onclick()
 * 
 * @brief: Date save onclick handler.
 * @param p_widget: pointer to a `widget_t` structure
 **/

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


/**
 * settings_invert_onclick()
 * 
 * @brief: Screen rotation button onclick handler.
 * @param p_widget: pointer to a `widget_t` structure
 **/

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


/**
 * settings_one_tile_event_handler()
 * 
 * @brief: First setting screen event handler
 * @param p_tile: pointer to a tile
 * @param event: tile event
 * @param x: x-coord for the event
 * @param y: y-coord for the event
 * @param velocity: swipe speed (if swipe event)
 **/

int settings_one_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Get date and time. */
        twatch_rtc_get_date_time(&datetime);

        /* Update time. */
        timeset_set_time(&timeset, &datetime);

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


/**
 * settings_two_tile_event_handler()
 * 
 * @brief: Second setting screen event handler
 * @param p_tile: pointer to a tile
 * @param event: tile event
 * @param x: x-coord for the event
 * @param y: y-coord for the event
 * @param velocity: swipe speed (if swipe event)
 **/

int settings_two_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Get date and time. */
        twatch_rtc_get_date_time(&datetime);
        uint8_t year = datetime.year%100;

        /* Update date. */
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

/**
 * tile_settings_one_init()
 * 
 * @brief: Initialize our first setting screen (time)
 **/

tile_t *tile_settings_one_init(void)
{
  /* Get date and time. */
  twatch_rtc_get_date_time(&datetime);

  /* Initialize our tile. */
  tile_init(&settings_one_tile, NULL);
  tile_set_event_handler(&settings_one_tile, settings_one_tile_event_handler);

  /* Initialize our title label. */
  widget_label_init(&lbl_one_title, &settings_one_tile, 10, 5, 230, 45, "Settings 1/3");

  timeset_init(&timeset, &settings_one_tile, 0, 60, 0, 0);

  /* Get hours and minutes and set timeset. */
  timeset_set_time(&timeset, &datetime);

  /* Initialize our modal box (confirm). */
  modal_init(&confirm, 20, 80, 200, 120);
  confirm.tile.background_color = RGB(0x0, 0x8, 0xc);
  widget_button_init(
    &confirm_btn,
    &confirm.tile,
    50,
    80,
    100,
    30,
    "OK"
  );
  widget_set_bg_color(&confirm_btn.widget, RGB(0xe, 0xe, 0xe));
  widget_set_front_color(&confirm_btn.widget, RGB(0, 0, 0));

  widget_button_set_handler(&confirm_btn, clock_save_confirm);
  widget_label_init(
    &confirm_txt,
    &confirm.tile,
    10,
    5,
    180,
    30,
    "Time saved !"
  );
  widget_set_bg_color(&confirm_txt.widget, RGB(0x0, 0x8, 0xc));
  widget_set_front_color(&confirm_txt.widget, RGB(0xf, 0xf, 0xf));
  

  /* Initialize our buttons. */
  widget_button_init(&btn_save_clock, &settings_one_tile, 15, 190, 210, 45, "Save");
  widget_set_bg_color(&btn_save_clock.widget, RGB(0x0, 0x8, 0xc));
  widget_set_border_color(&btn_save_clock.widget, RGB(0x0, 0x8, 0xc));
  widget_button_set_handler(&btn_save_clock, clock_save_onclick);

  /* Return our tile. */
  return &settings_one_tile;
}


/**
 * tile_settings_two_init()
 * 
 * @brief: Initialize our second setting screen (date)
 **/

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
  widget_set_bg_color(&btn_save_date.widget, RGB(0x0, 0x8, 0xc));
  widget_set_border_color(&btn_save_date.widget, RGB(0x0, 0x8, 0xc));
  widget_button_set_handler(&btn_save_date, date_save_onclick);

  /* Return our tile. */
  return &settings_two_tile;
}

void backlight_onchanged(widget_slider_t *p_widget_slider/*, int new_value, int old_value*/)
{
  int new_value = widget_slider_get_value(p_widget_slider);
  int before = twatch_screen_get_backlight();
  if (before != new_value) {
    twatch_screen_set_backlight(new_value);
    widget_slider_set_value(p_widget_slider, new_value);
  }
}

/**
 * tile_settings_three_init()
 * 
 * @brief: Initialize our thord setting screen (screen rotation)
 **/

tile_t *tile_settings_three_init(void)
{
  /* Initialize our tile. */
  tile_init(&settings_three_tile, NULL);

  /* Initialize our title label. */
  widget_label_init(&lbl_three_title, &settings_three_tile, 10, 5, 230, 45, "Settings 3/3");
  
  widget_button_init(&btn_orientation, &settings_three_tile, 15, (240-45)/2, 210, 45, "Rotate screen");
  widget_set_bg_color(&btn_orientation.widget, RGB(0x0, 0x8, 0xc));
  widget_set_border_color(&btn_orientation.widget, RGB(0x0, 0x8, 0xc));
  widget_button_set_handler(&btn_orientation, settings_invert_onclick);

  /* Initialize our backlight slider */
  widget_slider_init(&sld_backlight, &settings_three_tile, 2, 200, 240-2, 20);
  widget_slider_configure(&sld_backlight, 10, 5000, SCREEN_DEFAULT_BACKLIGHT, -1);
  widget_slider_set_handler(&sld_backlight, backlight_onchanged);

  /* Return our tile. */
  return &settings_three_tile;
}
