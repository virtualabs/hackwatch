#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#include "sound.h"

int sound_init(sound_system_t *p_sound_system, int sample_rate)
{
  esp_err_t err;

  /* Sanity check. */
  if (p_sound_system == NULL)
    return SOUND_ERR;

  /* Reset sound system structure. */
  memset(p_sound_system, 0, sizeof(sound_system_t));

  /**
   * Initialize the sound system.
   **/

  p_sound_system->port = DEFAULT_I2S_PORT;
  p_sound_system->sample_rate = sample_rate;

  /* Initialize ESP32 I2S config. */
  p_sound_system->ss_config.mode = I2S_MODE_MASTER | I2S_MODE_TX;
  p_sound_system->ss_config.sample_rate = sample_rate;
  p_sound_system->ss_config.bits_per_sample = 16;
  p_sound_system->ss_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  p_sound_system->ss_config.communication_format = I2S_COMM_FORMAT_STAND_MSB;

  /* Use ESP_INTR_FLAG_LEVEL2, as ESP_INTR_FLAG_LEVEL1 is already used. */
  p_sound_system->ss_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL2;
  p_sound_system->ss_config.dma_buf_count = 6;
  p_sound_system->ss_config.dma_buf_len = 60;
  p_sound_system->ss_config.use_apll = false;

  /* Initialize ESP32 pin config. */
  p_sound_system->ss_pin_config.bck_io_num = 26;
  p_sound_system->ss_pin_config.ws_io_num = 25;
  p_sound_system->ss_pin_config.data_out_num = 33;
  //p_sound_system->ss_pin_config.data_in_num = -1;

  /* Install I2S driver. */
  err = i2s_driver_install(
    p_sound_system->port,
    &p_sound_system->ss_config,
    0,
    NULL
  );
  if (err != ESP_OK)
  {
    /* Error while installing I2S driver. */
    ESP_LOGE("sound_system", "cannot install i2s driver: %d\r\n", err);
    return SOUND_ERR;
  }

  err = i2s_set_pin(p_sound_system->port, &p_sound_system->ss_pin_config);
  if (err != ESP_OK)
  {
    /* Error while setting pins. */
    ESP_LOGE("sound_system", "setting i2s pins failed: %d\r\n", err);
    return SOUND_ERR;
  }

  /* Success. */
  return SOUND_OK;
}

int sound_send_samples(sound_system_t *p_sound_system, void *samples, size_t samples_size, size_t *p_bytes_written, TickType_t ticks_to_wait)
{
  esp_err_t err;
  err = i2s_write(
    p_sound_system->port,
    samples,
    samples_size,
    p_bytes_written,
    ticks_to_wait
  );

  return (err == ESP_OK)?SOUND_OK:SOUND_ERR;
}

int sound_deinit(sound_system_t *p_sound_system)
{
  esp_err_t err;

  if (p_sound_system != NULL)
  {
    err = i2s_driver_uninstall(p_sound_system->port);
    if (err != ESP_OK)
    {
      /* Error, cannot uninstall driver. */
      ESP_LOGE("sound_system", "cannot uninstall driver: %d\r\n", err);
      return SOUND_ERR;
    }

    /* Success. */
    return SOUND_OK;
  }
  else
    return SOUND_ERR;
}
