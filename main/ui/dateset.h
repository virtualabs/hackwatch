#ifndef __INC_WIDGET_DATESET_H
#define __INC_WIDGET_DATESET_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/widget.h"

#define DATESET_WIDTH 240
#define DATESET_HEIGHT 120

typedef struct {
  /* Base widget. */
  widget_t base_widget;

  /* Font. */
  image_t *p_font;

  /* Day, month and year. */
  int day;
  int month;
  int year;

  /* Press and longpress */
  int last_x;
  int last_y;
  int press_cycles;
  int nb_cycles;
  bool longpress;

} widget_dateset_t;

void dateset_init(widget_dateset_t *p_dateset, tile_t *p_tile, int x, int y, int day, int month, int year);
void dateset_set_date(widget_dateset_t *p_dateset, rtc_datetime_t *p_datetime);
void dateset_get_date(widget_dateset_t *p_dateset, rtc_datetime_t *p_datetime);

#endif /* __INC_WIDGET_DATESET_H */