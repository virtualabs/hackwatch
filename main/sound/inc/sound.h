#ifndef __INC_SOUND_H
#define __INC_SOUND_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"


#define SOUND_OK  1
#define SOUND_ERR 0
#define DEFAULT_I2S_PORT  0

typedef struct t_sound_system {
  i2s_config_t      ss_config;
  i2s_pin_config_t  ss_pin_config;
  int               sample_rate;
  int               port;
} sound_system_t;

/* Initialize 16-bit sound system. */
int sound_init(sound_system_t *p_sound_system, int sample_rate);

/* Send samples to sound system. */
int sound_send_samples(sound_system_t *p_sound_system, void *samples, size_t samples_size, size_t *p_bytes_written, TickType_t ticks_to_wait);

/* Deinitialize 16-bit sound system. */
int sound_deinit(sound_system_t *p_sound_system);


#endif /* __INC_SOUND_H */
