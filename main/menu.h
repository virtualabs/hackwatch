#ifndef __INC_MENU_H
#define __INC_MENU_H

#include "twatch.h"

typedef tile_t* (*Ftile_init)(void);

tile_t *menu_add_menu(tile_t *p_tile_prev, tile_t *p_tile);
tile_t *menu_add_tile(tile_t *p_menu_tile, tile_t *p_tile_prev, Ftile_init pfn_tile_init);

#endif /* __INC_MENU_H */