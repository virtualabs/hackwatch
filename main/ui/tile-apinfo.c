#include "tile-apinfo.h"

tile_t apinfo_tile;
widget_label_t apinfo_lbl;

#define TAG "[tile::wifi::apinfo]"

tile_t *tile_apinfo_init(void)
{
  /* Initialize our tile. */
  tile_init(&apinfo_tile, NULL);

  /* Add a label. */
  widget_label_init(&apinfo_lbl, &apinfo_tile, 80, 110, 220, 45, "AP Info");


  /* Return our tile. */
  return &apinfo_tile;
}