#include "dateset.h"
#include "../img/digits_small.h"
#include "../img/ticks.h"

#define DIGIT_WIDTH   28
#define DIGIT_HEIGHT  45

#define DAY_X       18
#define DAY_Y       0
#define MONTH_X     (DIGIT_WIDTH*2 + 36)
#define MONTH_Y     0
#define YEAR_X      (DIGIT_WIDTH*4 + 54)
#define YEAR_Y      0

static image_t *p_font;
static image_t *p_ticks;

/* Months (obviously). */
enum {
  JAN=1,
  FEB,
  MAR,
  APR,
  MAY,
  JUN,
  JUL,
  AUG,
  SEP,
  OCT,
  NOV,
  DEC
};


/**
 * is_leap_year()
 * 
 * @brief: determine if a given year is leap
 * @param year: year to test
 * @return: true if it is a leap year, false otherwise
 **/

bool is_leap_year(int year)
{
  return (
    ( ((year % 4) == 0) && (year % 100 != 0) ) ||
    ( year % 400 == 0)
  ) && (year>0);
}


/**
 * nb_days_in_month()
 * 
 * @brief: compute the number of days in a given month of a year
 * @param month: month number (starting from 1)
 * @param year: year
 * @return: number of days in the month
 **/

int nb_days_in_month(int month, int year)
{
  if (month == FEB)
  {
    if (is_leap_year(year))
      return 29;
    else
      return 28;
  }
  else
  {
    return ((((month-1)%7)%2)==0)?31:30;
  }
}


/**
 * nb_days_in_month()
 * 
 * @brief: compute the number of days in a given month of a year
 * @param month: month number (starting from 1)
 * @param year: year
 * @return: number of days in the month
 **/

void dateset_fix(widget_dateset_t *p_dateset)
{
  int days_in_month;

  /* Determine the number of days in the current month. */
  days_in_month = nb_days_in_month(p_dateset->month, p_dateset->year);

  /* Limit day number depending on the month. */
  if (p_dateset->day >= days_in_month)
  {
    p_dateset->day = days_in_month;
  }
  if (p_dateset->day <= 1)
  {
    p_dateset->day = 1;
  }

  /* Limit month. */
  if (p_dateset->month < JAN)
    p_dateset->month = JAN;
  else if (p_dateset->month > DEC)
    p_dateset->month = DEC;
}


/**
 * _dateset_drawfunc()
 * 
 * @brief: dateset widget drawing function
 * @param p_widget: pointer to a `widget_t` structure
 * @return: TE_PROCESSED
 **/

int _dateset_drawfunc(widget_t *p_widget)
{
  int delta_y;

  widget_dateset_t *p_dateset = (widget_dateset_t *)p_widget->p_user_data;

  delta_y = (p_widget->box.height - 45)/2;

  /* Draw day digit 1. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_dateset->day/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    DAY_X,
    delta_y + DAY_Y
  );
  
  /* Draw day digit 2. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_dateset->day%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    DAY_X + DIGIT_WIDTH,
    delta_y + DAY_Y
  );

  /* Draw month digit 1. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_dateset->month/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    MONTH_X,
    delta_y + MONTH_Y
  );
  
  /* Draw month digit 2. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_dateset->month%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    MONTH_X + DIGIT_WIDTH,
    delta_y + MONTH_Y
  );

  /* Draw year digit 1. */
  widget_bitblt(
    p_widget,
    p_font,
    ((p_dateset->year%100)/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    YEAR_X,
    delta_y + YEAR_Y
  );
  
  /* Draw year digit 2. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_dateset->year%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    YEAR_X + DIGIT_WIDTH,
    delta_y + YEAR_Y
  );


  /* Draw upper triangles. */
  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    0,
    26,
    13,
    DAY_X + DIGIT_WIDTH/2 + 1,
    delta_y - 18
  );

  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    0,
    26,
    13,
    MONTH_X + DIGIT_WIDTH/2 + 1,
    delta_y - 18
  );

  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    0,
    26,
    13,
    YEAR_X + DIGIT_WIDTH/2,
    delta_y - 18
  );

  /* Draw lower triangles. */
  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    13,
    26,
    13,
    DAY_X + DIGIT_WIDTH/2 + 1,
    delta_y + DIGIT_HEIGHT + 5
  );

  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    13,
    26,
    13,
    MONTH_X + DIGIT_WIDTH/2 + 1,
    delta_y + DIGIT_HEIGHT + 5
  );

  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    13,
    26,
    13,
    YEAR_X + DIGIT_WIDTH/2 + 1,
    delta_y + DIGIT_HEIGHT + 5
  );

  /* Draw day/month/year texts. */
  widget_draw_text(p_widget, DAY_X + DIGIT_WIDTH - font_get_text_width("day")/2, 100, "day", RGB(0xc,0xc,0xc));
  widget_draw_text(p_widget, MONTH_X + DIGIT_WIDTH - font_get_text_width("month")/2, 100, "month", RGB(0xc,0xc,0xc));
  widget_draw_text(p_widget, YEAR_X + DIGIT_WIDTH - font_get_text_width("year")/2, 100, "year", RGB(0xc,0xc,0xc));

  /* Success. */
  return TE_PROCESSED;
}


/**
 * update_date()
 * 
 * @brief: Update dateset date
 * @param p_widget: pointer to a `widget_t` structure
 * @param x: X coordinate of a press event
 * @param y: Y coordinate of a press event
 * @param longpress: true if event is a long press, false otherwise
 **/

void update_date(widget_t *p_widget, int x, int y, bool longpress)
{
  widget_dateset_t *p_dateset = (widget_dateset_t *)p_widget->p_user_data;  


  /* Is day pressed ? */
  if ((x >= DAY_X) && (x <= (DAY_X + DIGIT_WIDTH*2)))
  {
    /* Update day value. */
    if (y > (p_widget->box.height)/2)
    {
      p_dateset->day -= (longpress?2:1);
    }
    else
    {
      p_dateset->day += (longpress?2:1);
    }

    /* Fix value if needed. */
    dateset_fix(p_dateset);
  }
  
  /* Is month pressed ? */
  if ((x >= MONTH_X) && (x <= (MONTH_X + DIGIT_WIDTH*2)))
  {
    /* Update day value. */
    if (y > (p_widget->box.height)/2)
    {
      p_dateset->month -= (longpress?2:1);
    }
    else
    {
      p_dateset->month += (longpress?2:1);
    }

    /* Fix value if needed. */
    dateset_fix(p_dateset);
  }

  /* Is year pressed ? */
  if ((x >= YEAR_X) && (x <= (YEAR_X + DIGIT_WIDTH*2)))
  {
    /* Update day value. */
    if (y > (p_widget->box.height)/2)
    {
      p_dateset->year--;
    }
    else
    {
      p_dateset->year++;
    }

    /* Fix value if needed. */
    dateset_fix(p_dateset);
  }
}


/**
 * _dateset_eventhandler()
 * 
 * @brief: Dateset widget event handler
 * @param p_widget: pointer to a `widget_t` structure
 * @param event: event type
 * @param x: X coordinate of a press event
 * @param y: Y coordinate of a press event
 * @param velocity: speed of a swipe event
 * @return WE_PROCESSED if success.
 **/

int _dateset_eventhandler(widget_t *p_widget, widget_event_t p_event, int x, int y, int velocity)
{
  bool b_event_processed = false;
  widget_dateset_t *p_dateset = (widget_dateset_t *)p_widget->p_user_data;  

  switch(p_event)
  {
    case WE_PRESS:
      {
        p_dateset->last_x = x;
        p_dateset->last_y = y;

        p_dateset->press_cycles++;
        if (p_dateset->press_cycles > 20)
        {
          if (!p_dateset->longpress)
          {
            p_dateset->nb_cycles++;
            if (p_dateset->nb_cycles > 5)
            {
              p_dateset->longpress = true;
            }
          }

          /* Update time. */
          update_date(p_widget, x, y, p_dateset->longpress);

          /* Reset cycles. */
          p_dateset->press_cycles = 0;
        }
      }
      break;

    case WE_RELEASE:
      {
        if (p_dateset->press_cycles > 2)
        {
          /* Update time. */
          update_date(p_widget, p_dateset->last_x, p_dateset->last_y, p_dateset->longpress);
        }

        /* Reset press cycles. */
        p_dateset->press_cycles = 0;
        p_dateset->nb_cycles = 0;
        p_dateset->longpress = false;
      }
      break;

    default:
      break;
  }

  /* Return WE_PROCESSED if event has been processed. */
  return (b_event_processed?WE_PROCESSED:WE_ERROR);
}


/**
 * dateset_set_date()
 * 
 * @brief: Update dateset widget date
 * @param p_dateset: pointer to a `widget_dateset_t` structure
 * @param p_datetime: pointer to a `rtc_datetime_t` structure
 **/

void dateset_set_date(widget_dateset_t *p_dateset, rtc_datetime_t *p_datetime)
{
  /* Set day, month and year. */
  p_dateset->day = p_datetime->day;
  p_dateset->month = p_datetime->month;
  p_dateset->year = p_datetime->year;
}


/**
 * dateset_get_date()
 * 
 * @brief: Retrieve dateset widget date
 * @param p_dateset: pointer to a `widget_dateset_t` structure
 * @param p_datetime: pointer to a `rtc_datetime_t` structure
 **/

void dateset_get_date(widget_dateset_t *p_dateset, rtc_datetime_t *p_datetime)
{
  /* Copy day, month and year into p_datetime. */
  p_datetime->day = p_dateset->day;
  p_datetime->month = p_dateset->month;
  p_datetime->year = p_dateset->year;
}


/**
 * dateset_init()
 * 
 * @brief: Initialize a dateset widget
 * @param p_dateset: pointer to a `widget_dateset_t` structure
 * @param p_tile: pointer to a `tile_t` structure
 * @param x: widget X coordinate
 * @param y: widget Y coordinate
 * @param day: current day
 * @param month: current month
 * @param year: current year
 **/

void dateset_init(widget_dateset_t *p_dateset, tile_t *p_tile, int x, int y, int day, int month, int year)
{
  /* Initialize our widget. */
  widget_init(&p_dateset->base_widget, p_tile, x, y, DATESET_WIDTH, DATESET_HEIGHT);
  widget_set_drawfunc(&p_dateset->base_widget, _dateset_drawfunc);
  widget_set_eventhandler(&p_dateset->base_widget, _dateset_eventhandler);
  widget_set_userdata(&p_dateset->base_widget, (void *)p_dateset);

  /* Load our font. */
  p_font = load_image(digits_small_img);

  /* Load our ticks. */
  p_ticks = load_image(ticks_img);
  
  /* Set day, month and year. */
  p_dateset->day = day;
  p_dateset->month = month;
  p_dateset->year = year;
  dateset_fix(p_dateset);

  p_dateset->nb_cycles = 0;
  p_dateset->longpress = false;
}