#ifndef __INC_HACKWATCH_BLEINFO_H
#define __INC_HACKWATCH_BLEINFO_H

#include "twatch.h"
#include "ui/ui.h"
#include "ui/frame.h"
#include "../ble/ble.h"

modal_t *modal_bleinfo_init(void);
void modal_bleinfo_clear(void);
void modal_bleinfo_set_title(ble_device_t *p_device);
void modal_bleinfo_update(uint8_t ble_ver, uint16_t comp_id, uint16_t sw_ver);
void modal_bleinfo_wait(void);

#endif /* __INC_HACKWATCH_BLEINFO_H */