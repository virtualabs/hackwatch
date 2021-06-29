
#include "tile-channels.h"

static tile_t channels_tile;
static int g_ticks = 0;
static int g_channel = 1;
static int g_pkt_count = 0;
static int g_measures[CHANNELS_MAX_CHANS*CHANNELS_NB_MEASURES];

#define TAG "[tile::wifi::channels]"

void channels_add_measure(int channel, int rssi)
{
  int i;

  for (i=(channel - 1)*CHANNELS_NB_MEASURES + 1; i<(channel*CHANNELS_NB_MEASURES); i++)
  {
    g_measures[i - 1] = g_measures[i];
  }
  g_measures[i-1] = rssi;
}

int channels_get_avg_measure(int channel)
{
  int i, avg = 0;
  for (i=(channel - 1)*CHANNELS_NB_MEASURES; i<(channel*CHANNELS_NB_MEASURES); i++)
  {
    avg += g_measures[i];
  }

  avg = avg/CHANNELS_NB_MEASURES;

  if (avg < -100)
    avg = -100;

  if (avg >= 0)
    avg = 0;

  return avg;
}

void channels_on_pkt_sniffed(wifi_promiscuous_pkt_t *p_wifi_pkt)
{
  //printf("got packet on channel %d (%d)\r\n", g_channel, p_wifi_pkt->rx_ctrl.rssi);
  /* We received a packet. */
  g_pkt_count++;

  /* Insert our RSSI. */
  channels_add_measure(g_channel, p_wifi_pkt->rx_ctrl.rssi);
}

int channels_draw(tile_t *p_tile)
{
  char *tableau[] = {"1","2","3","4","5","6","7","8","9","","11","","","14"};
  int i, avg, height;

  /* Set drawing window. */
  st7789_set_drawing_window(
    p_tile->offset_x,
    p_tile->offset_y,
    p_tile->offset_x + p_tile->width,
    p_tile->offset_y + p_tile->height
  );

  /* Fill region with background color. */
  st7789_fill_region(
    p_tile->offset_x,
    p_tile->offset_y,
    p_tile->width,
    p_tile->height,
    p_tile->background_color
  );

  /* Draw title. */
  tile_draw_text_x2(p_tile, 3, 3, "Channel scan", RGB(0xf,0xf,0xf));

  /* Draw channel labels (source: rebelor49) */
  for (i=0; i<14; i++)
  {
    if (tableau[i][0] != 0)
    {
      tile_draw_text(p_tile, (i>9)?(14 + i*16 - 2):(14 + i*16), 220, tableau[i], RGB(0xb,0xb,0xb));
    }
  }

  /* Draw a frame */
  tile_draw_line(
    p_tile,
    3,
    39,
    240-6,
    39,
    RGB(0x0, 0x8, 0xc)
  );
  tile_draw_line(
    p_tile,
    2,
    40,
    2,
    210,
    RGB(0x0, 0x8, 0xc)
  );
  tile_draw_line(
    p_tile,
    3,
    211,
    240-6,
    211,
    RGB(0x0, 0x8, 0xc)
  );
  tile_draw_line(
    p_tile,
    240-6,
    40,
    240-6,
    210,
    RGB(0x0, 0x8, 0xc)
  );

  /* Draw rules. */
  for (i=0; i<100; i+=10)
  {
    tile_draw_line(
      p_tile,
      3,
      CHANNELS_TABLE_Y + (CHANNELS_COLUMN_HEIGHT - (i*CHANNELS_COLUMN_HEIGHT)/100),
      240-6,
      CHANNELS_TABLE_Y + (CHANNELS_COLUMN_HEIGHT - (i*CHANNELS_COLUMN_HEIGHT)/100),
      RGB(0x3,0x3,0x3)
    );
  }

  /* Draw our measures. */
  for (i=0; i<CHANNELS_MAX_CHANS; i++)
  {
    tile_draw_line(
      p_tile,
      CHANNELS_TABLE_X + i*CHANNELS_COLUMN_WIDTH + CHANNELS_COLUMN_WIDTH/2,
      40,
      CHANNELS_TABLE_X + i*CHANNELS_COLUMN_WIDTH + CHANNELS_COLUMN_WIDTH/2,
      CHANNELS_TABLE_Y + CHANNELS_COLUMN_HEIGHT + 5,
      RGB(0x3,0x3,0x3)
    );

    avg = channels_get_avg_measure(i+1);
    height = ((avg + 100) * CHANNELS_COLUMN_HEIGHT)/100;
    if (height > 0)
    {
      tile_fill_region(
        p_tile,
        i*CHANNELS_COLUMN_WIDTH + CHANNELS_TABLE_X + 2,
        CHANNELS_TABLE_Y + (CHANNELS_COLUMN_HEIGHT - height),
        CHANNELS_COLUMN_WIDTH - 2,
        height,
        RGB(0xf,0xf,0xf)
      );
    }
  }


  /* Increment ticks, update channel if required. */
  g_ticks++;
  if (g_ticks > CHANNELS_SCAN_TIMEOUT_TICKS)
  {
    /* Display current measures for channel. */
    printf("Measures chan %d: ", g_channel);
    for (int j=(g_channel-1)*CHANNELS_NB_MEASURES; j<g_channel*CHANNELS_NB_MEASURES; j++)
    {
      printf("%d ", g_measures[j]);
    }
    printf("\r\n");

    /* No packets received ? Add a new measure of value -100. */
    if (g_pkt_count == 0)
    {
      channels_add_measure(g_channel, -100);
    }

    /* Switch to next channel. */
    g_ticks = 0;
    g_pkt_count = 0;
    g_channel = (g_channel + 1);
    if (g_channel > CHANNELS_MAX_CHANS)
      g_channel = 1;
    wifi_set_channel(g_channel);
  }

  /* Success. */
  return 0;
}

int channels_tile_event_handler(tile_t *p_tile, tile_event_t event, int x, int y, int velocity)
{
  switch (event)
  {
    case TE_ENTER:
      {
        /* Disable eco mode. */
        disable_ecomode();

        /* Enable scanner. */
        wifi_set_mode(WIFI_SNIFFER);

        /* Set channel. */
        g_channel = 1;
        wifi_set_channel(g_channel);

        /* Register our packet sniffer callback. */
        wifi_set_sniffer_handler(channels_on_pkt_sniffed);

        g_ticks = 0;
        g_pkt_count = 0;
      }
      break;

    case TE_EXIT:
      {
        /* Stop scanner. */
        wifi_set_mode(WIFI_OFF);

        /* Unregister our packet sniffer callback. */
        wifi_set_sniffer_handler(NULL);

        /* Re-enable eco mode. */
        enable_ecomode();
      }
      break;

    default:
      break;
  }

  /* Success. */
  return TE_PROCESSED;
}

tile_t *tile_channels_init(void)
{
  int i;

  /* Initialize our tile. */
  tile_init(&channels_tile, NULL);
  tile_set_drawfunc(&channels_tile, channels_draw);
  tile_set_event_handler(&channels_tile, channels_tile_event_handler);

  /* Initialize our measures. */
  for (i=0; i<(CHANNELS_NB_MEASURES * CHANNELS_MAX_CHANS); i++)
    g_measures[i] = -100;

  /* Return our tile. */
  return &channels_tile;
}