#ifndef __INC_HACKWATCH_SETTINGS_H
#define __INC_HACKWATCH_SETTINGS_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/frame.h"

typedef struct {
  widget_label_t label;

  /* Properties. */
  int min_value;
  int max_value;
  int current_value;

  /* Label text buffer. */
  char psz_label_text[3];
  
} widget_value_select_t;

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
);
int widget_value_select_get_value(widget_value_select_t *p_value_select);

tile_t *tile_settings_one_init(void);
tile_t *tile_settings_two_init(void);
tile_t *tile_settings_three_init(void);

#endif /* __INC_HACKWATCH_SETTINGS_H */