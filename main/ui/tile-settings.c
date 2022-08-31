#include "tile-settings.h"

static tile_t settings_one_tile, settings_two_tile, settings_three_tile;

/* Time and date saved confirm modal */
static modal_t confirm, confirm_date;
static widget_button_t confirm_btn, confirm_date_btn;
static widget_label_t confirm_txt, confirm_date_txt;

static rtc_datetime_t datetime;
static widget_label_t lbl_one_title, lbl_two_title, lbl_three_title;
static widget_button_t btn_orientation;
static widget_button_t btn_save_clock, btn_save_date;
static widget_timeset_t timeset;
static widget_dateset_t dateset;
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
 * clock_save_onclick()
 * 
 * @brief: time save button callback handler
 * @param p_widget: pointer to a `widget_t` structure
 **/

int clock_save_onclick(widget_t *p_widget)
{
  twatch_rtc_get_date_time(&datetime);
  timeset_get_time(&timeset, &datetime);
  twatch_rtc_set_date_time(&datetime);

  ui_set_modal(&confirm);

  /* Success. */
  return TE_PROCESSED;
}


/**
 * clock_save_confirm()
 * 
 * @brief: time save confirm modal handler
 * @param p_widget: pointer to a `widget_t` structure
 **/

int clock_save_confirm(widget_t *p_widget)
{
  ui_unset_modal();

  /* Success. */
  return TE_PROCESSED;
}


/**
 * date_save_onclick()
 * 
 * @brief: Date save onclick handler.
 * @param p_widget: pointer to a `widget_t` structure
 **/

int date_save_onclick(widget_t *p_widget)
{
  /* Set datetime. */
  twatch_rtc_get_date_time(&datetime);
  dateset_get_date(&dateset, &datetime);
  twatch_rtc_set_date_time(&datetime);

  ui_set_modal(&confirm_date);

  /* Success. */
  return TE_PROCESSED;
}


/**
 * date_save_confirm()
 * 
 * @brief: date save confirm modal handler
 * @param p_widget: pointer to a `widget_t` structure
 **/

int date_save_confirm(widget_t *p_widget)
{
  ui_unset_modal();

  /* Success. */
  return TE_PROCESSED;
}


/**
 * settings_invert_onclick()
 * 
 * @brief: Screen rotation button onclick handler.
 * @param p_widget: pointer to a `widget_t` structure
 **/

int settings_invert_onclick(widget_t *p_widget)
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

  /* Success. */
  return TE_PROCESSED;
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
        dateset_set_date(&dateset, &datetime);
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
  confirm.tile.background_color = RGB(0x1, 0x2, 0x3);
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
  widget_set_bg_color(&confirm_txt.widget, RGB(0x1, 0x2, 0x3));
  widget_set_front_color(&confirm_txt.widget, RGB(0xf, 0xf, 0xf));
  

  /* Initialize our buttons. */
  widget_button_init(&btn_save_clock, &settings_one_tile, 15, 190, 210, 45, "Save");
  widget_set_bg_color(&btn_save_clock.widget, RGB(0x1, 0x2, 0x3));
  widget_set_border_color(&btn_save_clock.widget, RGB(0x1, 0x2, 0x3));
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

  /* Initialize our tile. */
  tile_init(&settings_two_tile, NULL);
  tile_set_event_handler(&settings_two_tile, settings_two_tile_event_handler);

  /* Initialize our title label. */
  widget_label_init(&lbl_two_title, &settings_two_tile, 10, 5, 230, 45, "Settings 2/3");

  dateset_init(&dateset, &settings_two_tile, 0, 50, datetime.day, datetime.month, datetime.year);
  
  /* Initialize our modal box (confirm_date). */
  modal_init(&confirm_date, 20, 80, 200, 120);
  confirm_date.tile.background_color = RGB(0x1, 0x2, 0x3);
  widget_button_init(
    &confirm_date_btn,
    &confirm_date.tile,
    50,
    80,
    100,
    30,
    "OK"
  );
  widget_set_bg_color(&confirm_date_btn.widget, RGB(0xe, 0xe, 0xe));
  widget_set_front_color(&confirm_date_btn.widget, RGB(0, 0, 0));

  widget_button_set_handler(&confirm_date_btn, date_save_confirm);
  widget_label_init(
    &confirm_date_txt,
    &confirm_date.tile,
    10,
    5,
    180,
    30,
    "Date saved !"
  );
  widget_set_bg_color(&confirm_date_txt.widget, RGB(0x1, 0x2, 0x3));
  widget_set_front_color(&confirm_date_txt.widget, RGB(0xf, 0xf, 0xf));

  /* Initialize our buttons. */
  widget_button_init(&btn_save_date, &settings_two_tile, 15, 190, 210, 45, "Save date");
  widget_set_bg_color(&btn_save_date.widget, RGB(0x1, 0x2, 0x3));
  widget_set_border_color(&btn_save_date.widget, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&btn_save_date, date_save_onclick);

  /* Return our tile. */
  return &settings_two_tile;
}

/**
 * backlight_onchanged()
 * 
 * @brief: Callback for backlight value changed
 * @param p_widget_slider: pointer to a `widget_t` structure
 * @return TE_PROCESSED if event has been processed.
 **/

int backlight_onchanged(widget_t *p_widget_slider)
{
  int new_value = widget_slider_get_value((widget_slider_t *)p_widget_slider);
  int before = twatch_screen_get_backlight();

  /* Update value if it has changed. */
  if (before != new_value) {
    twatch_screen_set_default_backlight(new_value);
    twatch_screen_set_backlight(new_value);
    widget_slider_set_value((widget_slider_t *)p_widget_slider, new_value);
  }

  /* Success. */
  return TE_PROCESSED;
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
  widget_set_bg_color(&btn_orientation.widget, RGB(0x1, 0x2, 0x3));
  widget_set_border_color(&btn_orientation.widget, RGB(0x1, 0x2, 0x3));
  widget_button_set_handler(&btn_orientation, settings_invert_onclick);

  /* Initialize our backlight slider */
  widget_slider_init(&sld_backlight, &settings_three_tile, 2, 180, 240-4, 60);
  widget_slider_configure(&sld_backlight, 10, 5000, twatch_screen_get_default_backlight(), -1);
  widget_slider_set_handler(&sld_backlight, backlight_onchanged);

  /* Return our tile. */
  return &settings_three_tile;
}
