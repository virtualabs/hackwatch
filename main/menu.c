#include "menu.h"

tile_t *menu_add_menu(tile_t *p_tile_prev, tile_t *p_tile)
{  
  /* Link this tile to the previous one (if any). */
  if (p_tile_prev != NULL)
    tile_link_right(p_tile_prev, p_tile);

  return p_tile;
}

tile_t *menu_add_tile(tile_t *p_menu_tile, tile_t *p_tile_prev, Ftile_init pfn_tile_init)
{
  tile_t *p_tile = pfn_tile_init();
  tile_link_bottom(p_menu_tile, p_tile);
  
  /* Link this tile to the previous one (if any). */
  if (p_tile_prev != NULL)
    tile_link_right(p_tile_prev, p_tile);

  return p_tile;
}
