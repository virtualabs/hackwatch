#ifndef __INC_HACKWATCH_APINFO_H
#define __INC_HACKWATCH_APINFO_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/frame.h"
#include "../wifi/wifi.h"

tile_t *tile_apinfo_init(void);
void tile_apinfo_set_ap(wifi_ap_t *p_wifi_ap);

#endif /* __INC_HACKWATCH_APINFO_H */