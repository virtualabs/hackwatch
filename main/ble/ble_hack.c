#include "ble_hack.h"
#include "esp_bt.h"

/* Callback functions. */
FBLEHACK_IsrCallback gpfn_on_rx_data_pdu = NULL;
FBLEHACK_IsrCallback gpfn_on_rx_control_pdu = NULL;
FBLEHACK_IsrCallback gpfn_on_tx_data_pdu = NULL;
FBLEHACK_ProgPacketsCallback gpfn_on_tx_prog = NULL;
FBLEHACK_RawCallback gpfn_on_raw_packet = NULL;

/* Structure storing radio configuration parameters. */
radio_config_t radio_config;

/* Global bluetooth platform log level. */
extern int g_bt_plf_log_level;

/* RX buffer free function. */
extern uint16_t r_em_buf_rx_free(uint32_t desc);

/* Rx handler hooking (ISR). */
uint8_t *p_rx_buffer = (uint8_t *)(BLE_RX_BUFFER_ADDR);
typedef int (*F_lld_pdu_rx_handler)(int param_1,int param_2);
F_lld_pdu_rx_handler pfn_lld_pdu_rx_handler = NULL;

/* Hook r_lld_pdu_tx_prog(struct lld_evt_tag *evt) */
typedef int (*F_r_lld_pdu_tx_prog)(struct lld_evt_tag *evt);
F_r_lld_pdu_tx_prog pfn_lld_pdu_tx_prog = NULL;

/* Hook r_lld_pdu_data_tx_push(struct lld_evt_tag *evt, struct em_desc_node *txnode, bool can_be_freed, bool encrypted) */
typedef int (*F_r_lld_pdu_data_tx_push)(struct lld_evt_tag *evt, struct em_desc_node *txnode, bool can_be_freed);
F_r_lld_pdu_data_tx_push pfn_lld_pdu_data_tx_push = NULL;

typedef int (*F_r_llm_set_scan_en)(uint8_t *scan_param);
F_r_llm_set_scan_en pfn_llm_start_scan_en = NULL;

volatile bool gb_busy = false;
volatile int g_step = 0;

esp_packet_processed_t packets[8];

/* External BLE controller callback functions structure. */
extern void *r_btdm_option_data[1548];
void *g_ip_funcs_p = (void *)(IP_FUNCS_ARRAY_ADDR);
extern void **r_ip_funcs_p[981];
extern void **r_modules_funcs_p[100];

extern struct em_buf_env_tag em_buf_env;
struct co_list *gp_tx_prog = NULL;
extern struct lld_evt_tag lld_evt_env;
struct em_desc_node* pkt = NULL;
extern uint32_t *llc_env[10];

extern uint32_t r_llc_util_get_nb_active_link(void);
extern void r_em_buf_tx_free(struct em_buf_node *node);
int rx_custom_config_esp32(uint8_t *scan_param);

/* ANT */
uint8_t g_raw_payload[ANT_PAYLOAD_MAX_SIZE + 6];

void disable_interrupts(void)
{
    portDISABLE_INTERRUPTS();
    esp_rom_printf("Interrupts disabled\n");
}

void enable_interrupts(void)
{
    portENABLE_INTERRUPTS();
    esp_rom_printf("Interrupts enabled\n");
}

uint32_t switch_endianness(uint32_t val) {
	/* Switch endianness of a uint32_t value. */
   uint32_t ret = ((val & 0x000000FF) << 24) |
                  ((val & 0x0000FF00) << 8) |
                  ((val & 0x00FF0000) >> 8) |
                  ((val & 0xFF000000) >> 24);
   return ret;
}

uint8_t swap(uint8_t val) {
		/* Swap a byte. */
   val = (val & 0xF0) >> 4 | (val & 0x0F) << 4;
   val = (val & 0xCC) >> 2 | (val & 0x33) << 2;
   val = (val & 0xAA) >> 1 | (val & 0x55) << 1;
   return val;
}

void dewhiten_ble(uint8_t *data, int len, int channel) {
		/* Dewhiten a BLE packet according to a specific channel. */
    int i,j;
    uint8_t c;
    uint8_t lfsr = swap(channel) | 2;

    for (i=0; i<len; i++)
    {
        c = swap(data[i]);
        for (j=7; j>=0; j--)
        {
            if (lfsr & 0x80)
            {
                lfsr ^= 0x11;
                c ^= (1<<j);
            }
            lfsr <<= 1;
        }
        data[i] = swap(c);
    }
}


unsigned short crc16(uint8_t *ptr, int count, int init) {
		/* Perform a CRC computation with a given data and init value (polynomial: 0x1021). */
   int  crc;
   uint8_t i;
   crc = init;
   while (--count >= 0)
   {
      crc = crc ^ (int) *ptr++ << 8;
      i = 8;
      do
      {
         if (crc & 0x8000)
            crc = crc << 1 ^ 0x1021;
         else
            crc = crc << 1;
      } while(--i);
   }
   return (crc);
}



/**
 * co_list_is_empty()
 * 
 * Checks if a co_list structure is empty.
 **/

bool co_list_is_empty(const struct co_list *const list)
{
    bool listempty;
    listempty = (list->first == NULL);
    return (listempty);
}


/**
 * co_list_pop_front()
 * 
 * Pops the first item of the list, return NULL if any.
 **/

struct co_list_hdr *co_list_pop_front(struct co_list *list)
{
    struct co_list_hdr *element;

    // check if list is empty
    element = list->first;
    if (element != NULL)
    {

        // The list isn't empty : extract the first element
        list->first = list->first->next;

        if(list->first == NULL)
        {
            list->last = list->first;
        }

        list->cnt--;
        if(list->mincnt > list->cnt)
        {
            list->mincnt = list->cnt;
        }
    }
    return element;
}


/**
 * co_list_push_back()
 * 
 * Appends an item at the end of a given list.
 **/

void co_list_push_back(struct co_list *list,
                       struct co_list_hdr *list_hdr)
{
    // check if list is empty
    if (co_list_is_empty(list))
    {
        // list empty => pushed element is also head
        list->first = list_hdr;
    }
    else
    {
        // list not empty => update next of last
        list->last->next = list_hdr;
    }

    // add element at the end of the list
    list->last = list_hdr;
    list_hdr->next = NULL;

    list->cnt++;
    if(list->maxcnt < list->cnt)
    {
        list->maxcnt = list->cnt;
    }
}

void *r_emi_get_mem_addr_by_offset(uint16_t offset) {
    return (void*)(BLE_RX_BUFFER_ADDR + offset);
}

void set_packet_length(int packet_num, uint8_t length)
{
    pkt_hdr_t *p_header = (pkt_hdr_t *)(BLE_RX_PKT_HDR_ADDR);
    uint32_t pkt_header;
    
    /* Read packet header from fifo header (located at 0x3ffb094c). */
    pkt_header = p_header[packet_num].header;

    /* Update length. */
    pkt_header = (pkt_header & 0xffff00ff) | ((length)<<8);

    /* Overwrite header. */
    p_header[packet_num].header = pkt_header;
}


/**
 * _lld_pdu_rx_handler()
 * 
 * Hook for r_lld_pdu_rx_handler(), called each time a BLE packet (header + PDU) is received.
 **/

int _lld_pdu_rx_handler(int param_1,int param_2)
{
  int forward = HOOK_FORWARD;
  uint16_t *p_offset;
  uint8_t fifo_index;
  uint32_t pkt_header;
  uint8_t *p_pdu;
  int pkt_size;
  pkt_hdr_t *p_header = (pkt_hdr_t *)(BLE_RX_PKT_HDR_ADDR);
  uint16_t pkt_status = *(uint16_t *)(BLE_RX_PKT_STATUS);
  int i,j,k;
  int proc_pkt_idx = 0;

  /**
   * If we are called with multiple packets, we need to temporarily allocate
   * memory to reorder packets and put the forwarded ones to the end. So first
   * we call our hooks, then we rewrite the RX descriptors and some internal
   * structures and eventually return to the normal execution flow. 
   */

  if (gb_busy)
  {
    esp_rom_printf("[rx handler] already busy (%d)\n", g_step);
    pfn_lld_pdu_rx_handler(param_1, param_2);
    return 0;
  }

  gb_busy = true;
    
  /* We retrieve the fifo index from memory. */
  fifo_index = ((uint8_t *)BLE_RX_CUR_FIFO_ADDR)[FIFO_INDEX_OFFSET];

  /* 0. If we don't have any packet to process, just forward. */
  g_step = 0;
  if ((param_2 == 0) || ((pkt_status & 0x13f) != 0))
  {
    /* Re-enable interrupts. */
    //portENABLE_INTERRUPTS();

    /* Forward to original function. */
    gb_busy = false;
    
    return pfn_lld_pdu_rx_handler(param_1, param_2);
  }

  /* BLE_RX_DESC_ADDR -> array of 12-byte items, the first 2 are offsets. */
  /* p_rx_buffer -> BLE RX/TX shared memory. */

  if ((*((uint8_t *)param_1 + 0x72) & 0x10) == 0)
  {
    if (r_llc_util_get_nb_active_link() > 0)
    {
        //esp_rom_printf("param1: 0x%08x, param2: %d\n", (uint32_t)param_1, param_2);
        //esp_rom_printf("Current SW FIFO index: %d\n", *((uint32_t *)(0x3ffb933c)));

        /* 1. We parse all the packets and moved them into dynamically allocated structures. */
        g_step = 1;
        for (k=0; k<param_2; k++)
        {
            j = (fifo_index + k) % 8;

            /* Read packet header from fifo header (located at 0x3ffb094c). */
            pkt_header = p_header[j].header;
            
            /* Extract channel, rssi and packet size. */
            pkt_size = (pkt_header >> 8) & 0xff;

            /* Fill current RX packet. */
            p_offset = (uint16_t *)(BLE_RX_DESC_ADDR + 12*j);
            p_pdu = (uint8_t *)(p_rx_buffer + *p_offset);
            packets[proc_pkt_idx].b_forward = false;
            packets[proc_pkt_idx].header = pkt_header;
            packets[proc_pkt_idx].length = pkt_size;
            //packets[proc_pkt_idx].pdu = p_pdu;

            if (pkt_size > 0)
            {
                memcpy(packets[proc_pkt_idx].pdu, p_pdu, pkt_size);

                /* Call our hook (if any) in case of a control PDU. */
                if ((pkt_header & 0x03) == 0x3)
                {
                    if (gpfn_on_rx_control_pdu != NULL)
                    {
                        //esp_rom_printf("call rx ctl handler\n");
                        forward = gpfn_on_rx_control_pdu(
                            j,
                            (uint16_t)(pkt_header & 0xffff),
                            packets[proc_pkt_idx].pdu,
                            packets[proc_pkt_idx].length
                        );

                        /* Should we forward this packet ? */
                        packets[proc_pkt_idx].b_forward = (forward == HOOK_FORWARD);
                    }
                }
                
                /* Or call our other hook (if any) in case of a data PDU. */
                else if ((pkt_header & 0x03) != 0)
                {
                    if (gpfn_on_rx_data_pdu != NULL)
                    {
                        //esp_rom_printf("call rx data handler (%d)\n", pkt_size);
                        forward = gpfn_on_rx_data_pdu(
                            j,
                            (uint16_t)(pkt_header & 0xffff),
                            packets[proc_pkt_idx].pdu,
                            packets[proc_pkt_idx].length
                        );

                        /* Should we forward this packet ? */
                        packets[proc_pkt_idx].b_forward = (forward == HOOK_FORWARD);
                    }
                }
            }
            g_step++;

            /* Increment rx pkt index. */
            proc_pkt_idx++;
        }

        /* 2. Ok, now we have a list of `proc_pkt_idx` processed packets, and we need to
        * write them back into ESP32 FIFOs. The main idea is to count the number of skipped
        * packets, increment DWORD @0x3ffb933c used by ESP32-WROOM to keep the current
        * RX FIFO index for every skipped packet, and rewrite the forwarded packets into
        * the last RX FIFOs.
        **/
        g_step = 10;
        for (i=0; i<proc_pkt_idx; i++)
        {
            j = (fifo_index + i) % 8;

            if (!packets[i].b_forward)
            {
                /* Turn packet into an empty packet. */
                packets[i].header &= 0xffff0000;
                packets[i].header |= 0xffff0002;

                /* Update header. */
                p_header[j].header = packets[i].header;
            }
            else
            {
                if (packets[i].length > 0)
                {
                    /* Copy RAM into PDU. */
                    p_offset = (uint16_t *)(BLE_RX_DESC_ADDR + 12*j);
                    p_pdu = (uint8_t *)(p_rx_buffer + *p_offset);
                    memcpy(p_pdu, packets[i].pdu, packets[i].length);            
                }
            }
        }
    }
  }

  gb_busy = false;

  /* Forward to original handler. */
  return pfn_lld_pdu_rx_handler(param_1, param_2);
}

/**
 * @brief _lld_raw_rx_handler
 * 
 * @param evt           pointer to a BLE event
 * @param nb_rx_desc    number of RX descriptors to process
 * @return int          original result
 */
int _lld_raw_rx_handler(void* evt, uint32_t nb_rx_desc) {
  pkt_hdr_t *p_pkt = (pkt_hdr_t *)(r_emi_get_mem_addr_by_offset(HEADER_IN_EM_OFFSET));
  uint16_t pkt_status = *(uint16_t *)r_emi_get_mem_addr_by_offset(PACKET_STATUS_IN_EM_OFFSET);
  if (nb_rx_desc == 0 || ((pkt_status & 0x13f) != 0)) {
      return pfn_lld_pdu_rx_handler((int) evt, (int) nb_rx_desc);
  }
  int j;
  if ((*((uint8_t *)evt + 0x72) & 0x10) == 0) {
    uint8_t fifo_index = ((uint8_t *)BLE_RX_CUR_FIFO_ADDR)[FIFO_INDEX_OFFSET];

    for (int k=0; k<nb_rx_desc; k++) {

      j = (fifo_index + k) % 8;
      uint32_t pkt_header = p_pkt[j].header;

      int8_t rssi = (pkt_header>>16) & 0xff;
      uint8_t pkt_size = (pkt_header >> 8) & 0xff;
      if (pkt_size > 0 && pkt_size < 255) {
        uint8_t opcode = pkt_header & 0xFF;

        uint16_t *buffer_offset = (uint16_t*)(r_emi_get_mem_addr_by_offset(RX_DESCRIPTOR_IN_EM_OFFSET) + 12*j);
        uint8_t *p_pdu = (uint8_t *)(r_emi_get_mem_addr_by_offset(*buffer_offset));
        
        memcpy(g_raw_payload, &radio_config.sync_word, 4);
        g_raw_payload[4] = opcode;
        g_raw_payload[5] = pkt_size;
        memcpy(&g_raw_payload[6],p_pdu, pkt_size);

        if (radio_config.swapping) {
          for (int i=0;i<pkt_size;i++) g_raw_payload[i] = swap(g_raw_payload[i]);
        }
        g_raw_payload[4] = g_raw_payload[4] ^ (1 << 3);

        if (gpfn_on_raw_packet != NULL)
        {
          gpfn_on_raw_packet(g_raw_payload, pkt_size + 6, rssi, radio_config.frequency);
        }
        
      }
      p_pkt[j].header = 0;
    }
  }

    /* Forward to original handler. */
  return pfn_lld_pdu_rx_handler((int)evt, (int)nb_rx_desc);
}

/**
 * _lld_pdu_tx_prog()
 * 
 * This hook is called during each connection event to handle pending BLE PDUs
 * (data PDU + control PDU).
 **/

int _lld_pdu_tx_prog(struct lld_evt_tag *evt)
{
  int res;

  /* Notify our callback. */
  if (gpfn_on_tx_prog != NULL)
  {
    gpfn_on_tx_prog();
  }

  /* Call tx prog. */
  res = pfn_lld_pdu_tx_prog(evt);

  return res;
}


/**
 * _lld_pdu_data_tx_push()
 * 
 * This hook is called during each connection event to handle pending BLE PDUs
 * (data PDU + control PDU).
 **/

int _lld_pdu_data_tx_push(struct lld_evt_tag *evt, struct em_desc_node *txnode, bool can_be_freed)
{
  int res;
  uint8_t *p_buf = (uint8_t *)(p_rx_buffer + txnode->buffer_ptr);

  if (gpfn_on_tx_data_pdu != NULL)
  {
    /* Should we block this data PDU ? */
    if (gpfn_on_tx_data_pdu(-1, txnode->llid | (txnode->length << 8), p_buf, txnode->length) == HOOK_BLOCK)
    {
      /* Set TX buffer length to zero (won't be transmitted, but will be freed later. */
      txnode->length = 0;
    }
  }

  /* Call data tx push. */
  res = pfn_lld_pdu_data_tx_push(evt, txnode, can_be_freed);
  return res;
}


/**
 * em_buf_tx_alloc()
 * 
 * Copy of a ROM function that allocates a TX node.
 **/

struct em_buf_node *em_buf_tx_alloc(void)
{
    struct em_buf_node *node = NULL;
    portDISABLE_INTERRUPTS();
    // Get free element from free list
    node = (struct em_buf_node *) co_list_pop_front(&em_buf_env.tx_buff_free);
    portENABLE_INTERRUPTS();
    return node;
}

struct em_desc_node *em_buf_tx_desc_alloc(void)
{
    struct em_desc_node *node = NULL;
    portDISABLE_INTERRUPTS();
    node = (struct em_desc_node *) co_list_pop_front(&em_buf_env.tx_desc_free);
    portENABLE_INTERRUPTS();
    return node;
}


/**
 * send_raw_data_pdu()
 * 
 * @brief Sends a raw data PDU. 
 * @param conhdl: connection handle (by default 0 if there is only one connection alive)
 * @param p_pdu: pointer to a data PDU (bytes)
 * @param length: data PDU length (without its header)
 **/

void IRAM_ATTR send_raw_data_pdu(int conhdl, uint8_t llid, uint8_t *p_pdu, int length, bool can_be_freed)
{
  struct em_buf_node* node;
  struct em_desc_node *data_send;
  struct lld_evt_tag *env = (struct lld_evt_tag *)(*(uint32_t*)((uint32_t)llc_env[conhdl]+0x10) + 0x28);

  /* Allocate data_send. */
  data_send = (struct em_desc_node *)em_buf_tx_desc_alloc();

  /* Allocate a buffer. */
  node = em_buf_tx_alloc();

  /* Write data into allocated buf node. */
  memcpy((uint8_t *)((uint8_t *)p_rx_buffer + node->buf_ptr), p_pdu, length);

  /* Write information into our em_desc_node structure. */
  data_send->llid = llid;
  data_send->length = length;
  data_send->buffer_idx = node->idx;
  data_send->buffer_ptr = node->buf_ptr;

  /* Call lld_pdu_data_tx_push */  
  pfn_lld_pdu_data_tx_push(env, data_send, can_be_freed);
}

/**
 * ble_hack_install_hooks()
 * 
 * Install our hooks into ESP32's BLE controller memory.
 **/

void ble_hack_install_hooks(void)
{
  /* Hook r_lld_pdu_rx_handler() */
  pfn_lld_pdu_rx_handler = (void *)(r_btdm_option_data[615]);
  r_btdm_option_data[615] = (uint32_t *)_lld_pdu_rx_handler;

  /* Hook r_lld_pdu_data_tx_push */
  pfn_lld_pdu_data_tx_push = (void *)(((uint32_t *)g_ip_funcs_p)[597]);
  ((uint32_t *)g_ip_funcs_p)[597] = (uint32_t)_lld_pdu_data_tx_push;
}


/**
 * @brief Set BLE hack Data PDU callback.
 * @param pfn_data_callback: pointer to a callback that'll be notified each time a data PDU is received.
 **/

void ble_hack_rx_data_pdu_handler(FBLEHACK_IsrCallback pfn_data_callback)
{
  gpfn_on_rx_data_pdu = pfn_data_callback;
}


/**
 * @brief Set BLE hack Control PDU callback.
 * @param pfn_data_callback: pointer to a callback that'll be notified each time a control PDU is received.
 **/

void ble_hack_rx_control_pdu_handler(FBLEHACK_IsrCallback pfn_control_callback)
{
  gpfn_on_rx_control_pdu = pfn_control_callback;
}

/**
 * @brief Set BLE hack Control PDU callback.
 * @param pfn_data_callback: pointer to a callback that'll be notified each time a data PDU is transmitted.
 **/

void ble_hack_tx_data_pdu_handler(FBLEHACK_IsrCallback pfn_data_callback)
{
  gpfn_on_tx_data_pdu = pfn_data_callback;
}

void ble_hack_raw_packet_handler(FBLEHACK_RawCallback pfn_raw_callback)
{
  gpfn_on_raw_packet = pfn_raw_callback;
}

/**
 * Raw Radio
 */

void set_data_rate(datarate_t data_rate) {
    radio_config.data_rate = data_rate;
}

void set_frequency(uint32_t frequency) {
    radio_config.frequency = frequency;
}

void set_sync_word(uint32_t sync_word) {
    radio_config.sync_word = sync_word;
}
void set_swapping(bool swapping) {
    radio_config.swapping = swapping;
}

void configure_sync_word(uint32_t sync) {
    if (radio_config.swapping) {
        sync =  swap((sync & 0xFF000000) >> 24) << 24 |
                swap((sync & 0x00FF0000) >> 16) << 16 |
                swap((sync & 0x0000FF00) >> 8) << 8 |
                swap(sync & 0x000000FF);
    }
    uint8_t *control_structure = r_emi_get_mem_addr_by_offset(CONTROL_STRUCTURE_IN_EM_OFFSET);
    control_structure[SYNC_REG_IN_CS_OFFSET] = (sync & 0xFF000000) >> 24;
    control_structure[SYNC_REG_IN_CS_OFFSET+1] = (sync & 0x00FF0000) >> 16;
    control_structure[SYNC_REG_IN_CS_OFFSET+2] = (sync & 0x0000FF00) >> 8;
    control_structure[SYNC_REG_IN_CS_OFFSET+3] = (sync & 0x000000FF);
    radio_config.sync_word = switch_endianness(sync);
}

void configure_format(uint8_t format) {
    uint8_t *control_structure = r_emi_get_mem_addr_by_offset(CONTROL_STRUCTURE_IN_EM_OFFSET);
    control_structure[CNTL_REG_IN_CS_OFFSET] = format;
}

void configure_max_rx_buffer_size(uint8_t max_size) {
    uint8_t *control_structure = r_emi_get_mem_addr_by_offset(CONTROL_STRUCTURE_IN_EM_OFFSET);
    control_structure[RXMAXBUF_REG_IN_CS_OFFSET] = max_size;
}

void disable_frequency_hopping() {
    uint8_t *control_structure = r_emi_get_mem_addr_by_offset(CONTROL_STRUCTURE_IN_EM_OFFSET);
    control_structure[HOPCTRL_REG_IN_CS_OFFSET+1] = 0;
    control_structure[HOPCTRL_REG_IN_CS_OFFSET] = 39;
}

void configure_frequency(uint32_t frequency) {
    uint8_t offset = frequency - 2402;
    uint8_t *frequency_table = r_emi_get_mem_addr_by_offset(FREQUENCY_TABLE_IN_EM_OFFSET);
    frequency_table[39] = offset;
    radio_config.frequency = frequency;
}

void disable_crc_checking() {
  RWBLECNTL = RWBLECNTL | (1 << 16)| (1 << 17);
}

void disable_whitening() {
  RWBLECNTL = RWBLECNTL | (1 << 18);
}

int rx_custom_config_esp32(uint8_t *scan_param) {
  int ret = pfn_llm_start_scan_en(scan_param);
  if (radio_config.mode == MODE_RX && radio_config.enabled) {
    configure_format(LLD_TEST_MODE_RX);
    configure_sync_word(radio_config.sync_word);// ant: 0x65a31778; // scan ant ? 0x555565a3
    configure_frequency(radio_config.frequency);
    disable_frequency_hopping();
    configure_max_rx_buffer_size(0xFF);

    disable_crc_checking();
    disable_whitening();
    radio_config.enabled = true;
  }
  return ret;
}

void ble_hack_enable_raw(void)
{
  if (r_btdm_option_data[615] == _lld_pdu_rx_handler)
  {
    /* Hook r_lld_pdu_rx_handler */
    r_btdm_option_data[615] = (uint32_t *)_lld_raw_rx_handler;
  }
  
  /* Hook r_llm_start_scan_en */
  pfn_llm_start_scan_en = (*r_ip_funcs_p)[RX_SCAN_CONFIG_INDEX];
  (*r_ip_funcs_p)[RX_SCAN_CONFIG_INDEX] = (void*)rx_custom_config_esp32;  
}

void ble_hack_disable_raw(void)
{

  /* Restore EM frequency table to legit value. */
  configure_frequency(2480);

  if (r_btdm_option_data[615] == _lld_raw_rx_handler)
  {
    /* Hook r_lld_pdu_rx_handler */
    r_btdm_option_data[615] = (uint32_t *)_lld_pdu_rx_handler;
  }

  /* Unhook r_llm_start_scan_en */
  (*r_ip_funcs_p)[RX_SCAN_CONFIG_INDEX] = pfn_llm_start_scan_en;
}
