#include "ble_hack.h"

/* Callback functions. */
FBLEHACK_IsrCallback gpfn_on_rx_data_pdu = NULL;
FBLEHACK_IsrCallback gpfn_on_rx_control_pdu = NULL;
//FBLEHACK_IsrCallback gpfn_on_tx_data_pdu = NULL;
FBLEHACK_CtlCallback gpfn_on_tx_control_pdu = NULL;

/* Global bluetooth platform log level */
extern int g_bt_plf_log_level;

/* RX buffer free function. */
extern uint16_t r_em_buf_rx_free(uint32_t desc);

/* Rx handler hooking (ISR). */
uint8_t *p_rx_buffer = (uint8_t *)(BLE_RX_BUFFER_ADDR);
typedef int (*F_lld_pdu_rx_handler)(int param_1,int param_2);
F_lld_pdu_rx_handler pfn_lld_pdu_rx_handler = NULL;

/* Hook r_lld_data_send() */
typedef int (*F_r_lld_pdu_data_send)(struct hci_acl_data_tx *param);
F_r_lld_pdu_data_send pfn_lld_pdu_data_send = NULL;

/* Hook r_lld_pdu_tx_prog(struct lld_evt_tag *evt) */
typedef int (*F_r_lld_pdu_tx_prog)(struct lld_evt_tag *evt);
F_r_lld_pdu_tx_prog pfn_lld_pdu_tx_prog = NULL;
struct llcp_pdu_tag *g_prev_llcp = NULL;

F_rom_llc_llcp_send pfn_rom_llc_llcp_send = (void*)(0x40043ed4);

/* TODO */

/* Declare LLCP original function types and global pointers */
ROM_HOOK(llc_llcp_version_ind_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_ch_map_update_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_pause_enc_req_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_pause_enc_rsp_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_enc_req_pdu_send, uint16_t conhdl, struct hci_le_start_enc_cmd *param)
ROM_HOOK(llc_llcp_enc_rsp_pdu_send, uint16_t conhdl, struct llcp_enc_req *param)
ROM_HOOK(llc_llcp_start_enc_rsp_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_reject_ind_pdu_send, uint16_t conhdl, uint8_t rej_opcode, uint8_t reason)
ROM_HOOK(llc_llcp_con_update_pdu_send, uint16_t conhdl, struct llcp_con_upd_ind *param)
ROM_HOOK(llc_llcp_con_param_req_pdu_send, uint16_t conhdl, struct llc_con_upd_req_ind *param)
ROM_HOOK(llc_llcp_con_param_rsp_pdu_send, uint16_t conhdl, struct llc_con_upd_req_ind *param)
ROM_HOOK(llc_llcp_feats_req_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_feats_rsp_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_start_enc_req_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_terminate_ind_pdu_send, uint16_t conhdl, uint8_t err_code)
ROM_HOOK(llc_llcp_unknown_rsp_send_pdu, uint16_t conhdl, uint8_t unk_type)
ROM_HOOK(llc_llcp_ping_req_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_ping_rsp_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_length_req_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_length_rsp_pdu_send, uint16_t conhdl)
ROM_HOOK(llc_llcp_tester_send, uint8_t conhdl, uint8_t length, uint8_t *data)

/* External BLE controller callback functions structure. */
extern void *r_btdm_option_data[1548];
void *g_ip_funcs_p = (void *)(IP_FUNCS_ARRAY_ADDR);
extern void **r_ip_funcs_p[981];
extern void **r_modules_funcs_p[100];

extern struct em_buf_env_tag em_buf_env;
struct co_list *gp_tx_prog = NULL;
struct lld_evt_tag *gp_evt = NULL;
struct em_desc_node* pkt = NULL;

/* Give access to llc_llcp_send() ROM function. */
typedef int (*F_llc_llcp_send)(uint8_t conhdl, void *param, uint8_t opcode);
F_llc_llcp_send llc_llcp_send = (F_llc_llcp_send)(LLC_LLCP_SEND_ADDR);

typedef void (*F_llc_llcp_tester_send)(uint8_t conhdl, uint8_t length, uint8_t *data);
F_llc_llcp_tester_send llc_llcp_tester_send = (F_llc_llcp_send)(LLC_LLCP_TESTER_SEND_ADDR);

extern uint32_t r_llc_util_get_nb_active_link(void);

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
  int channel;
  int pkt_size;
  int rssi;
  int nb_links;
  pkt_hdr_t *p_header = (pkt_hdr_t *)(BLE_RX_PKT_HDR_ADDR);
  #ifdef BLE_HACK_DEBUG
  int i;
  #endif

  /* BLE_RX_DESC_ADDR -> array of 12-byte items, the first 2 are offsets. */
  /* p_rx_buffer -> BLE RX/TX shared memory. */
  
  /* We retrieve the fifo index from memory. */
  fifo_index = ((uint8_t *)BLE_RX_CUR_FIFO_ADDR)[0x5c8];

  /* If we don't have any packet to process, just forward. */
  if (param_2 == 0)
  {
    return pfn_lld_pdu_rx_handler(param_1, param_2);
  }

  /* Read packet header from fifo header (located at 0x3ffb094c). */
  pkt_header = p_header[fifo_index].header;

  /* Extract channel, rssi and packet size. */
  channel = (pkt_header>>24);
  rssi = (pkt_header>>16) & 0xff;
  pkt_size = (pkt_header >> 8) & 0xff;

  if (pkt_size > 0)
  {
    p_offset = (uint16_t *)(BLE_RX_DESC_ADDR + 12*fifo_index);
    p_pdu = (uint8_t *)(p_rx_buffer + *p_offset);

    #ifdef BLE_HACK_DEBUG
    /* Display packet info. */
    esp_rom_printf("Header: %08x, fifo=%d\n", pkt_header, fifo_index);
    for (i=0; i<pkt_size; i++)
    {
      esp_rom_printf("%02x", p_pdu[i]);
    }
    esp_rom_printf("\n");
    #endif

    /* Maybe not the smarter way to do that ... */
    nb_links = r_llc_util_get_nb_active_link();
    if (nb_links > 0)
    {
      #ifdef BLE_HACK_DEBUG
      esp_rom_printf("%d active links\r\n", nb_links);
      #endif

      /* Is it a control PDU ? */
      if ((pkt_header & 0x03) == 0x3)
      {
        if (gpfn_on_rx_control_pdu != NULL)
        {
          forward = gpfn_on_rx_control_pdu(p_pdu, (pkt_header>>8)&0xff);
        }
      }
      /* Is it a data PDU ? */
      else if ((pkt_header & 0x03) != 0)
      {
        if (gpfn_on_rx_data_pdu != NULL)
        {
          forward = gpfn_on_rx_data_pdu(p_pdu, (pkt_header>>8)&0xff);
        }
      }

      if (forward == HOOK_FORWARD)
      {
        /* Forward to original handler. */
        return pfn_lld_pdu_rx_handler(param_1, param_2);
      }
      else
      {
        /* Modify original PDU to set its length to 0. */
        p_header[fifo_index].header &= 0xffff00ff;
        return pfn_lld_pdu_rx_handler(param_1, param_2);
      }
    }
  }

  /* Forward to original handler. */
  return pfn_lld_pdu_rx_handler(param_1, param_2);
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
  struct co_list_hdr *item;
  struct llcp_pdu_tag *node;


  /* Parse ready to send packet descriptors. */
  item = (struct co_list_hdr *)evt->tx_llcp_pdu_rdy.first;

  /* Call tx prog. */
  res = pfn_lld_pdu_tx_prog(evt);
  return res;
}

/*********************************************
 * Link-layer Control Procedures hooks
 ********************************************/

void _llc_llcp_version_ind_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_version_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_version_ind_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_VERSION_IND;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_version_ind_pdu_send)(conhdl);
}

void _llc_llcp_ch_map_update_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_ch_map_update_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_ch_map_update_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_CHANNEL_MAP_REQ;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_ch_map_update_pdu_send)(conhdl); 
}

void _llc_llcp_pause_enc_req_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_pause_enc_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_pause_enc_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_PAUSE_ENC_REQ;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_pause_enc_req_pdu_send)(conhdl);   
}

void _llc_llcp_pause_enc_rsp_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_pause_enc_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_pause_enc_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_PAUSE_ENC_RSP;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_pause_enc_rsp_pdu_send)(conhdl);  
}

void _llc_llcp_enc_req_pdu_send(uint16_t conhdl, struct hci_le_start_enc_cmd *param)
{
  int forward = HOOK_FORWARD;
  llcp_enc_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_enc_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_ENC_REQ;
    params.header.conhdl = conhdl;
    params.param = param;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_enc_req_pdu_send)(conhdl, param);  
}

void _llc_llcp_enc_rsp_pdu_send(uint16_t conhdl, struct llcp_enc_req *param)
{
  int forward = HOOK_FORWARD;
  llcp_enc_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_enc_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_ENC_RSP;
    params.header.conhdl = conhdl;
    params.param = param;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_enc_rsp_pdu_send)(conhdl, param);  
}

void _llc_llcp_start_enc_rsp_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_start_enc_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_start_enc_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_START_ENC_RSP;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_start_enc_rsp_pdu_send)(conhdl); 
}

void _llc_llcp_reject_ind_pdu_send(uint16_t conhdl, uint8_t rej_opcode, uint8_t reason)
{
  int forward = HOOK_FORWARD;
  llcp_reject_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_reject_ind_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_REJECT_IND;
    params.header.conhdl = conhdl;
    params.rej_opcode = rej_opcode;
    params.reason = reason;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_reject_ind_pdu_send)(conhdl, rej_opcode, reason);  
}


void _llc_llcp_con_update_pdu_send(uint16_t conhdl, struct llcp_con_upd_ind *param)
{
  int forward = HOOK_FORWARD;
  llcp_con_update_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_con_update_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_CONNECTION_UPDATE_REQ;
    params.header.conhdl = conhdl;
    params.param = param;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_con_update_pdu_send)(conhdl, param);  
}

void _llc_llcp_con_param_req_pdu_send(uint16_t conhdl, struct llc_con_upd_req_ind *param)
{
  int forward = HOOK_FORWARD;
  llcp_con_param_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_con_param_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_CONNECTION_PARAM_REQ;
    params.header.conhdl = conhdl;
    params.param = param;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_con_param_req_pdu_send)(conhdl, param);
}

void _llc_llcp_con_param_rsp_pdu_send(uint16_t conhdl, struct llc_con_upd_req_ind *param)
{
  int forward = HOOK_FORWARD;
  llcp_con_param_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_con_param_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_CONNECTION_PARAM_RSP;
    params.header.conhdl = conhdl;
    params.param = param;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_con_param_rsp_pdu_send)(conhdl, param);
}

void _llc_llcp_feats_req_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_feats_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_feats_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_FEATURE_REQ;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_feats_req_pdu_send)(conhdl);
}

void _llc_llcp_feats_rsp_pdu_send(uint32_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_feats_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_feats_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_FEATURE_RSP;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_feats_rsp_pdu_send)(conhdl);
}

void _llc_llcp_start_enc_req_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_start_enc_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_start_enc_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_START_ENC_REQ;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_start_enc_req_pdu_send)(conhdl); 
}

void _llc_llcp_terminate_ind_pdu_send(uint16_t conhdl, uint8_t err_code)
{
  int forward = HOOK_FORWARD;
  llcp_terminate_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_terminate_ind_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_TERMINATE_IND;
    params.header.conhdl = conhdl;
    params.err_code = err_code;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_terminate_ind_pdu_send)(conhdl, err_code); 
}

void _llc_llcp_unknown_rsp_send_pdu(uint16_t conhdl, uint8_t unk_type)
{
  int forward = HOOK_FORWARD;
  llcp_unknown_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_unknown_rsp_send_pdu() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_UNKNOWN_RSP;
    params.header.conhdl = conhdl;
    params.unk_type = unk_type;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_unknown_rsp_send_pdu)(conhdl, unk_type);   
}

void _llc_llcp_ping_req_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_ping_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_ping_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_PING_REQ;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_ping_req_pdu_send)(conhdl);    
}

void _llc_llcp_ping_rsp_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_ping_rsp_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_ping_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_PING_RSP;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_ping_rsp_pdu_send)(conhdl);    
}

void _llc_llcp_length_req_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_length_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_length_req_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_LENGTH_REQ;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_length_req_pdu_send)(conhdl);   
}

void _llc_llcp_length_rsp_pdu_send(uint16_t conhdl)
{
  int forward = HOOK_FORWARD;
  llcp_length_req_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_length_rsp_pdu_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_LENGTH_RSP;
    params.header.conhdl = conhdl;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_length_rsp_pdu_send)(conhdl);   
}

void _llc_llcp_tester_send(uint8_t conhdl, uint8_t length, uint8_t *data)
{
  int forward = HOOK_FORWARD;
  llcp_tester_send_params params;

  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("llc_llcp_tester_send() called\r\n");
  #endif

  if (gpfn_on_tx_control_pdu != NULL)
  {
    params.header.opcode = LL_TESTER_SEND;
    params.header.conhdl = conhdl;
    params.length = length;
    params.data = data;
    forward = gpfn_on_tx_control_pdu((llcp_opinfo *)&params);
  }

  if (forward == HOOK_FORWARD)
    HOOKFUNCPTR(llc_llcp_tester_send)(conhdl, length, data); 
}

/**
 * _lld_pdu_data_send()
 * 
 * This hook is called whenever the BLE stack sends a data PDU.
 **/

int _lld_pdu_data_send(struct hci_acl_data_tx *param)
{
  struct em_buf_tx_desc *p_desc = NULL;
  uint8_t *ptr_data;
  int i;
  struct co_list_hdr *tx_desc;
  struct em_desc_node *tx_node;
  
  #ifdef BLE_HACK_DEBUG
  esp_rom_printf("lld_pdu_data_send:\r\n");
  esp_rom_printf("  conn_handle: %d\r\n", param->conhdl);
  esp_rom_printf("  bufsize: %d\r\n", param->length);
  esp_rom_printf("  buffer->idx: %d\r\n", param->buf->idx);
  esp_rom_printf("  buffer->ptr: 0x%08x\r\n", param->buf->buf_ptr);
  esp_rom_printf("  buffer: 0x%08x\r\n", param->buf);
  esp_rom_printf(">> ");
  for (i=0; i<param->length; i++)
    {
      esp_rom_printf("%02x", ((uint8_t *)(p_rx_buffer + param->buf->buf_ptr))[i]);
    }
  esp_rom_printf("\r\n");
  #endif

  return pfn_lld_pdu_data_send(param);
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


/**
 * send_data_pdu()
 * 
 * @brief Sends a data PDU. 
 * @param conhdl: connection handle (by default 0 if there is only one connection alive)
 * @param p_pdu: pointer to a data PDU (bytes)
 * @param length: data PDU length (without its header)
 **/

void send_data_pdu(int conhdl, void *p_pdu, int length)
{
  struct em_buf_node* node;
  struct hci_acl_data_tx *data_send;

  /* Allocate data_send. */
  data_send = (struct hci_acl_data_tx *)malloc(sizeof(struct hci_acl_data_tx));

  /* Allocate a buffer. */
  node = em_buf_tx_alloc();
  #ifdef BLE_HACK_DEBUG
  printf("node: 0x%08x\r\n", (uint32_t)node);
  printf("node buf: 0x%08x\r\n", (uint32_t)node->buf_ptr);
  printf("target buf: 0x%08x\r\n", (uint32_t)(p_rx_buffer + node->buf_ptr));
  printf("buffer idx: %d\r\n", node->idx);
  printf("ppdu: 0x%08x\r\n", (uint32_t)p_pdu);
  #endif

  /* Write data into buffer. */
  data_send->conhdl = conhdl;
  data_send->pb_bc_flag = 2;
  data_send->length = length;
  data_send->buf = node;

  /* Write data into allocated buf node. */
  memcpy((uint8_t *)((uint8_t *)p_rx_buffer + node->buf_ptr), p_pdu, length);

  /* Call lld_pdu_data_send */
  pfn_lld_pdu_data_send(data_send);
}


void send_control_pdu(int conhdl, uint8_t *p_pdu, int length)
{
  /*
   * Sends control PDU through llc_llcp_send().
   *
   * second parameter points to control PDU parameters, while third parameter specifies the
   * control PDU opcode.
   **/
  llc_llcp_send(conhdl, p_pdu, p_pdu[0]);
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
  #ifdef BLE_HACK_DEBUG
  printf("Hooking function %08x with %08x\n", (uint32_t)pfn_lld_pdu_rx_handler, (uint32_t)_lld_pdu_rx_handler);
  #endif
  r_btdm_option_data[615] = (uint32_t *)_lld_pdu_rx_handler;

  /* Hook r_lld_pdu_data_send */
  pfn_lld_pdu_data_send = (void *)(((uint32_t *)g_ip_funcs_p)[598]);
  #ifdef BLE_HACK_DEBUG
  printf("Hooking function %08x with %08x\n", (uint32_t)pfn_lld_pdu_data_send, (uint32_t)_lld_pdu_data_send);
  #endif
  ((uint32_t *)g_ip_funcs_p)[598] = (uint32_t)_lld_pdu_data_send;

  /* Hook r_lld_pdu_tx_prog */
  pfn_lld_pdu_tx_prog = (void *)(((uint32_t *)g_ip_funcs_p)[600]);
  #ifdef BLE_HACK_DEBUG
  printf("Hooking function %08x with %08x\n", (uint32_t)pfn_lld_pdu_tx_prog, (uint32_t)_lld_pdu_tx_prog);
  #endif
  ((uint32_t *)g_ip_funcs_p)[600] = (uint32_t)_lld_pdu_tx_prog;

  /**
   * Install LLCP hooks
   **/
  INSTALL_HOOK(492, llc_llcp_version_ind_pdu_send)
  INSTALL_HOOK(493, llc_llcp_ch_map_update_pdu_send)
  INSTALL_HOOK(494, llc_llcp_pause_enc_req_pdu_send)
  INSTALL_HOOK(495, llc_llcp_pause_enc_rsp_pdu_send)
  INSTALL_HOOK(496, llc_llcp_enc_req_pdu_send)
  INSTALL_HOOK(497, llc_llcp_enc_rsp_pdu_send)
  INSTALL_HOOK(498, llc_llcp_start_enc_rsp_pdu_send)
  INSTALL_HOOK(499, llc_llcp_reject_ind_pdu_send)
  INSTALL_HOOK(500, llc_llcp_con_update_pdu_send)
  INSTALL_HOOK(501, llc_llcp_con_param_req_pdu_send)
  INSTALL_HOOK(502, llc_llcp_con_param_rsp_pdu_send)
  INSTALL_HOOK(503, llc_llcp_feats_req_pdu_send)
  INSTALL_HOOK(504, llc_llcp_feats_rsp_pdu_send)
  INSTALL_HOOK(505, llc_llcp_start_enc_req_pdu_send)
  INSTALL_HOOK(506, llc_llcp_terminate_ind_pdu_send)
  INSTALL_HOOK(507, llc_llcp_unknown_rsp_send_pdu)
  INSTALL_HOOK(508, llc_llcp_ping_req_pdu_send)
  INSTALL_HOOK(509, llc_llcp_ping_rsp_pdu_send)
  INSTALL_HOOK(510, llc_llcp_length_req_pdu_send)
  INSTALL_HOOK(511, llc_llcp_length_rsp_pdu_send)
  INSTALL_HOOK(512, llc_llcp_tester_send)

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
 * @param pfn_data_callback: pointer to a callback that'll be notified each time a control PDU is received.
 **/

void ble_hack_tx_control_pdu_handler(FBLEHACK_CtlCallback pfn_control_callback)
{
  gpfn_on_tx_control_pdu = pfn_control_callback;
}

int rom_llc_llcp_send(int conhdl, uint8_t *p_pdu, uint8_t opcode)
{
  return pfn_rom_llc_llcp_send(conhdl, p_pdu, opcode);
}