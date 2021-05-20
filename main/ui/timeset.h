#ifndef __INC_WIDGET_TIMESET_H
#define __INC_WIDGET_TIMESET_H

#include "ui/ui.h"
#include "ui/widget.h"

#define TIMESET_WIDTH 240
#define TIMESET_HEIGHT 120

typedef struct {
  /* Base widget. */
  widget_t base_widget;

  /* Font. */
  image_t *p_font;

  /* Hours and minutes. */
  int hours;
  int minutes;

  /* Press and longpress */
  int last_x;
  int last_y;
  int press_cycles;
  int nb_cycles;
  bool longpress;

} widget_timeset_t;

void timeset_init(widget_timeset_t *p_timeset, tile_t *p_tile, int x, int y, int hours, int minutes);

#endif /* __INC_WIDGET_TIMESET_H */