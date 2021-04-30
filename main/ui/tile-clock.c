#include "tile-clock.h"
#include "../img/digits.h"

#define HOURS_X       15
#define HOURS_Y       10
#define MINS_X        120
#define MINS_Y        (240-90-10)
#define DIGIT_WIDTH   56
#define DIGIT_HEIGHT  90

#define BLUE  RGB(0x5,0x9,0xf)

#define TAG "[tile::clock]"


static tile_t clock_tile;
static image_t *clock_digits;
static widget_label_t date_lbl;
rtc_datetime_t datetime;
int hours=12, mins=34;

volatile char *psz_date[14];

int _tile_clock_draw(tile_t *p_tile)
{
  /* Set drawing window. */
  st7789_set_drawing_window(
    p_tile->offset_x,
    p_tile->offset_y,
    p_tile->offset_x + SCREEN_WIDTH,
    p_tile->offset_y + SCREEN_HEIGHT
  );

  /* Fill region with background color. */
  st7789_fill_region(
    p_tile->offset_x,
    p_tile->offset_y,
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    p_tile->background_color
  );


  /* Draw digit. */
  tile_bitblt(
    p_tile,
    clock_digits,
    (hours/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    HOURS_X,
    HOURS_Y
  );

  /* Draw digit. */
  tile_bitblt(
    p_tile,
    clock_digits,
    (hours%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    HOURS_X + DIGIT_WIDTH,
    HOURS_Y
  );

  /* Draw digit. */
  tile_bitblt(
    p_tile,
    clock_digits,
    (mins/10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    MINS_X,
    MINS_Y
  );

  /* Draw digit. */
  tile_bitblt(
    p_tile,
    clock_digits,
    (mins%10)*DIGIT_WIDTH,
    0,
    DIGIT_WIDTH,
    DIGIT_HEIGHT,
    MINS_X + DIGIT_WIDTH,
    MINS_Y
  );

  /* Draw widgets. */
  tile_draw_widgets(p_tile);

  /* Success. */
  return 0;
}


void clock_update(void *parameter)
{
  while(true)
  {
    twatch_rtc_get_date_time(&datetime);
    hours = datetime.hour;
    mins = datetime.minute;

    snprintf(
      psz_date,
      14, 
      "%02d/%02d/%04d", 
      datetime.day, 
      datetime.month, 
      datetime.year
    );
    widget_label_set_text(&date_lbl, psz_date);

    vTaskDelay(300/portTICK_PERIOD_MS);
  }
}


tile_t *tile_clock_init(void)
{
  /* Initialize date */
  psz_date[0] = '\0';

  /* Load digits into memory. */
  clock_digits = load_image(digits_img);

  /* Initialize our tile. */
  tile_init(&clock_tile, NULL);

  /* Add labels */
  widget_label_init(&date_lbl, &clock_tile, (240-160)/2, 100, 160, 50, "01/01/1970");
  widget_set_front_color(&date_lbl, BLUE);

  /* Set tile drawing function. */
  tile_set_drawfunc(&clock_tile, _tile_clock_draw);

  /* Create update task. */
  xTaskCreate(clock_update, "clock_update", 10000, NULL, 1, NULL);

  /* Return our tile. */
  return &clock_tile;
}