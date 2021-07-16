#ifndef __INC_HACKWATCH_TVBGONE_H
#define __INC_HACKWATCH_TVBGONE_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/frame.h"
#include "ui/switch.h"
#include "drivers/ir/esp32_rmt_remotes.h"

tile_t *tile_tvbgone_init(void);
bool tvbgone_is_enabled(void);

#endif /* __INC_HACKWATCH_TVBGONE_H */