#include "timeset.h"
#include "../img/digits_small.h"
#include "../img/ticks.h"

#define DIGIT_WIDTH   28
#define DIGIT_HEIGHT  45
#define HOURS_X       0
#define HOURS_Y       0

#define MINS_X       (DIGIT_WIDTH*2 + 8)
#define MINS_Y       0

static image_t *p_font;
static image_t *p_ticks;


void _timeset_drawfunc(widget_t *p_widget)
{
  int delta_x, delta_y;

  widget_timeset_t *p_timeset = (widget_timeset_t *)p_widget->p_user_data;
  
  delta_x = (p_widget->box.width - 4*DIGIT_WIDTH - 8)/2;
  delta_y = (p_widget->box.height - 45)/2;

  /* Draw ':'. */
  widget_bitblt(
    p_widget,
    p_font,
    283,
    0,
    8,
    DIGIT_HEIGHT,
    delta_x + DIGIT_WIDTH*2,
    delta_y + 0
  );


  /* Draw hours digit 1. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_timeset->hours/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    delta_x + HOURS_X,
    delta_y + HOURS_Y
  );
  
  /* Draw hours digit 2. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_timeset->hours%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    delta_x + HOURS_X + DIGIT_WIDTH,
    delta_y + HOURS_Y
  );

  /* Draw hours digit 1. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_timeset->minutes/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    delta_x + MINS_X,
    delta_y + MINS_Y
  );
  

  /* Draw hours digit 2. */
  widget_bitblt(
    p_widget,
    p_font,
    (p_timeset->minutes%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    delta_x + MINS_X + DIGIT_WIDTH,
    delta_y + MINS_Y
  );

  /* Draw upper triangles. */
  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    0,
    26,
    13,
    delta_x + DIGIT_WIDTH/2 + 1,
    delta_y - 18
  );

  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    0,
    26,
    13,
    delta_x + DIGIT_WIDTH/2 + 1 + DIGIT_WIDTH*2 + 8,
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
    delta_x + DIGIT_WIDTH/2 + 1,
    delta_y + DIGIT_HEIGHT + 5
  );

  widget_bitblt(
    p_widget,
    p_ticks,
    0,
    13,
    26,
    13,
    delta_x + DIGIT_WIDTH/2 + 1 + DIGIT_WIDTH*2 + 8,
    delta_y + DIGIT_HEIGHT + 5
  );
}

void update_time(widget_t *p_widget, int x, int y, bool longpress)
{
  widget_timeset_t *p_timeset = (widget_timeset_t *)p_widget->p_user_data;  

  if (x > (p_widget->box.width/2))
  {
    if (y > (p_widget->box.height)/2)
    {
      p_timeset->minutes -= (longpress?10:1);
      if (p_timeset->minutes < 0)
        p_timeset->minutes = 59;
    }
    else
    {
      p_timeset->minutes+= (longpress?10:1);
      if (p_timeset->minutes >= 60)
        p_timeset->minutes = 0;
    }
  }
  else
  {
    if (y > (p_widget->box.height)/2)
    {
      p_timeset->hours-=(longpress?2:1);
      if (p_timeset->hours < 0)
        p_timeset->hours = 23;
    }
    else
    {
      p_timeset->hours+=(longpress?2:1);
      if (p_timeset->hours >= 24)
        p_timeset->hours = 0;
    }
  }
}

int _timeset_eventhandler(widget_t *p_widget, widget_event_t p_event, int x, int y, int velocity)
{
  bool b_event_processed = false;
  widget_timeset_t *p_timeset = (widget_timeset_t *)p_widget->p_user_data;  

  switch(p_event)
  {
    case WE_PRESS:
      {
        p_timeset->last_x = x;
        p_timeset->last_y = y;

        p_timeset->press_cycles++;
        if (p_timeset->press_cycles > 20)
        {
          if (!p_timeset->longpress)
          {
            p_timeset->nb_cycles++;
            if (p_timeset->nb_cycles > 5)
            {
              p_timeset->longpress = true;
            }
          }

          /* Update time. */
          update_time(p_widget, x, y, p_timeset->longpress);

          /* Reset cycles. */
          p_timeset->press_cycles = 0;
        }
      }
      break;

    case WE_RELEASE:
      {
        if (p_timeset->press_cycles > 2)
        {
          /* Update time. */
          update_time(p_widget, p_timeset->last_x, p_timeset->last_y, p_timeset->longpress);
        }

        /* Reset press cycles. */
        p_timeset->press_cycles = 0;
        p_timeset->nb_cycles = 0;
        p_timeset->longpress = false;
      }
      break;

    default:
      break;
  }

  /* Return WE_PROCESSED if event has been processed. */
  return (b_event_processed?WE_PROCESSED:WE_ERROR);
}


void timeset_set_time(widget_timeset_t *p_timeset, rtc_datetime_t *p_datetime)
{
  /* Set hours and minutes. */
  p_timeset->hours = p_datetime->hour;
  p_timeset->minutes = p_datetime->minute;
}

void timeset_get_time(widget_timeset_t *p_timeset, rtc_datetime_t *p_datetime)
{
  /* Copy hours and minutes into p_datetime. */
  p_datetime->hour = p_timeset->hours;
  p_datetime->minute = p_timeset->minutes;
}

void timeset_init(widget_timeset_t *p_timeset, tile_t *p_tile, int x, int y, int hours, int minutes)
{
  /* Initialize our widget. */
  widget_init(&p_timeset->base_widget, p_tile, x, y, TIMESET_WIDTH, TIMESET_HEIGHT);
  widget_set_drawfunc(&p_timeset->base_widget, _timeset_drawfunc);
  widget_set_eventhandler(&p_timeset->base_widget, _timeset_eventhandler);
  widget_set_userdata(&p_timeset->base_widget, (void *)p_timeset);

  /* Load our font. */
  p_font = load_image(digits_small_img);

  /* Load our ticks. */
  p_ticks = load_image(ticks_img);
  
  /* Set hours and minutes. */
  p_timeset->hours = 0;
  p_timeset->minutes = 0;
  p_timeset->press_cycles = 0;
  p_timeset->nb_cycles = 0;
  p_timeset->longpress = false;
}