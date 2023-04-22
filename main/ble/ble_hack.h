#ifndef __INC_ESP32_BLE_HACK_H
#define __INC_ESP32_BLE_HACK_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

/* Enable this if you want more verbose messages. */
//#define BLE_HACK_DEBUG        0

#define LLC_LLCP_SEND_ADDR              (0x40043ed4)
#define LLC_LLCP_TESTER_SEND_ADDR       (0x400445e4)
#define LLC_ENV_ADDR                    (0x3ffb96d0)
#define IP_FUNCS_ARRAY_ADDR             (0x3ffae70c)
#define BLE_RX_BUFFER_ADDR              (0x3ffb0000)
#define BLE_RX_PKT_STATUS               (0x3ffb094a)
#define BLE_RX_PKT_HDR_ADDR             (0x3ffb094c)
#define BLE_RX_CUR_FIFO_ADDR            (0x3ffb8d74)
#define BLE_RX_DESC_ADDR                (0x3ffb0950)
#define FREQUENCY_TABLE_IN_EM_OFFSET    (0x40)
#define RX_SCAN_CONFIG_INDEX            (641)
#define CONTROL_STRUCTURE_IN_EM_OFFSET  (0x43A)
#define FIFO_INDEX_OFFSET               (0x5c8)
#define HEADER_IN_EM_OFFSET             (0x94c)
#define PACKET_STATUS_IN_EM_OFFSET      (0x94a)
#define RX_DESCRIPTOR_IN_EM_OFFSET      (0x950)

#define CNTL_REG_IN_CS_OFFSET           (0)
#define SYNC_REG_IN_CS_OFFSET           (6)
#define HOPCTRL_REG_IN_CS_OFFSET        (16)
#define RXMAXBUF_REG_IN_CS_OFFSET       (34)
#define TXDESCPTR_REG_IN_CS_OFFSET      (22)

#define RWBLECNTL                 (*(uint32_t *)(0x3ff71200))
#define RWBLECONF                 (*(uint32_t *)(0x3ff71208))
#define BLE_INTCNTL               (*(uint32_t *)(0x3ff7120C))
#define BLE_INTSTAT               (*(uint32_t *)(0x3ff71210))
#define BLE_INTRAWSTAT            (*(uint32_t *)(0x3ff71214))
#define BLE_ERRORTYPESTAT         (*(uint32_t *)(0x3ff71260))

/* Test modes formats */
#define LLD_TEST_MODE_RX          0x1D
#define LLD_TEST_MODE_TX          0x1E

#define HOOK_FORWARD              0x1
#define HOOK_BLOCK                0x0

#define ANT_PAYLOAD_MAX_SIZE      17


/**
 * Common list header structure (required by co_list* functions).
 **/

struct co_list_hdr
{
    /// Pointer to next co_list_hdr
    struct co_list_hdr *next;
};

/**
 * Common list structure
 **/

struct co_list
{
    /// pointer to first element of the list
    struct co_list_hdr *first;
    /// pointer to the last element
    struct co_list_hdr *last;

    /// number of element in the list
    uint32_t cnt;
    /// max number of element in the list
    uint32_t maxcnt;
    /// min number of element in the list
    uint32_t mincnt;
};

/**
 * Event Arbiter Element
 **/

struct ea_elt_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct co_list_hdr hdr;

    /// Pointer on the next element linked to the current action
    struct ea_elt_tag *linked_element;

    /// Programming time in basetime (CLOCKN) absolute value
    uint32_t timestamp;

    /**
     * ASAP field contains the type and the limit
     * bit |31  30|   29   |28 27|26 ...................... 0|
     * def | TYPE | Parity | RFU | LIMIT (absolute time)     |
     *
     * Type:
     *  - 00: No ASAP
     *  - 01: ASAP no limit
     *  - 10: ASAP with limit
     *
     * Parity (only for ASAP requests):
     *  - 0: even slots
     *  - 1: odd slots
     *
     * Limit (only for ASAP LIMIT requests):
     *  - Absolute value in slots
     *  - The reservation can not cross over this limit
     */
    uint32_t asap_field;

    /// Minimum duration of the event or frame (in slots)
    uint16_t duration_min;

    /// Current priority
    uint8_t current_prio;
    /// Threshold1 for prevent stop
    uint8_t prev_stop_thr1;
    /// Threshold2 for prevent stop
    uint8_t prev_stop_thr2;
    /// Threshold for prevent start
    uint8_t prev_start_thr;

    /************************************************************************************
     * ISR CALLBACKS
     ************************************************************************************/

    /// Start event or frame call back function
    void (*ea_cb_prevent_start)(struct ea_elt_tag*);
    /// Prevent stop call back function
    void (*ea_cb_prevent_stop)(struct ea_elt_tag*);
    ///  event or frame canceling call back function
    void (*ea_cb_cancel)(struct ea_elt_tag*);

    /// BT/BLE Specific environment variable
    void *env;
};


/**
 * Synchronization counters
 **/

struct lld_evt_anchor
{
    /// Base time counter value of the latest found sync
    uint32_t basetime_cnt;
    /// Fine time counter value of the latest found sync
    uint16_t finetime_cnt;
    /// Event counter of the of the latest found sync
    uint16_t evt_cnt;
};

/**
 * Non connected event information
 **/

struct lld_non_conn
{
    /// Scan Window Size
    uint32_t window;
    /// Anchor timestamp
    uint32_t anchor;
    /// Event end time stamp
    uint32_t end_ts;
    /// use to know if it's an initiate procedure
    bool     initiate;
    /// use to know if connect request has been sent
    bool     connect_req_sent;
};

/**
 * Connected event information
 **/

struct lld_conn
{
    /// Synchronization Window Size (in us)
    uint32_t sync_win_size;
    /// SCA Drift (in us)
    uint32_t sca_drift;
    /// Instant of the next action (in events)
    uint16_t instant;
    /// Latency
    uint16_t latency;
    /// Event counter
    uint16_t counter;
    /// Number of connection events missed since last anchor point
    uint16_t missed_cnt;
    /// Minimum duration of the event or frame (in slots)
    uint16_t duration_dft;
    /// Update offset
    uint16_t update_offset;
    /// Effective maximum tx time
    uint16_t eff_max_tx_time;
    /// Effective maximum tx size
    uint16_t eff_max_tx_size;
    /// Update window size
    uint8_t  update_size;
    /// Describe the action to be done when instant occurs
    uint8_t  instant_action;
    /// Master sleep clock accuracy
    uint8_t  mst_sca;
    /// value of the latest More Data bit received
    uint8_t  last_md_rx;
    /// TX programmed packet counter
    uint8_t  tx_prog_pkt_cnt;
    /// Default RX windows offset
    uint8_t  rx_win_off_dft;
    /// Default RX windows path delay compensation
    uint8_t  rx_win_pathdly_comp;
    #if (BLE_2MBPS)
    /// TX phy to be programmed
    uint8_t  tx_phy;
    /// RX phy to be programmed
    uint8_t  rx_phy;
    #endif // (BLE_2MBPS)
    /// Wait for a sync due to a connection update
    bool wait_con_up_sync;
	
	uint32_t win_size_backup;  //us
};


/**
 * Event Arbiter interval structure
 **/

struct ea_interval_tag
{
    /// List element for chaining in the Interval list
    struct co_list_hdr hdr;
    /// Interval used
    uint16_t interval_used;
    /// Offset used
    uint16_t offset_used;
    /// Bandwidth used
    uint16_t bandwidth_used;
    /// Connection handle used
    uint16_t conhdl_used;
    /// Role used
    uint16_t role_used;
    /// Odd offset or even offset
    bool odd_offset;
    /// Link id
    uint16_t linkid;
};


/**
 * Link-layer Driver main structure (contains everything about LLD)
 **/

struct lld_evt_tag
{
    /// Information about the latest found synchronization
    struct lld_evt_anchor anchor_point;

    /// List of TX Data descriptors ready for transmission (i.e. not yet chained with the CS)
    struct co_list tx_acl_rdy;
    /// List of TX Data descriptors ready to be freed (i.e. already chained in the CS)
    struct co_list tx_acl_tofree;
    /// List of TX LLCP descriptors ready for transmission (i.e. not yet chained with the CS)
    struct co_list tx_llcp_pdu_rdy;
    /// List of TX LLCP descriptors programmed for transmission (i.e. chained with the CS)
    struct co_list tx_prog;

    /// Interval element pointer linked to this event
    struct ea_interval_tag* interval_elt;

    /// event information for connected and non-connected activity
    union lld_evt_info
    {
        /// Non connected event information
        struct lld_non_conn non_conn;

        /// Connected event information
        struct lld_conn conn;
    } evt;

    /// Connection Handle
    uint16_t conhdl;
    /// Control structure pointer address
    uint16_t cs_ptr;
    /// Connection Interval
    uint16_t interval;
    /// Number of RX Descriptors already handled in the event
    uint8_t rx_cnt;
    /// Mode of the link (Master connect, slave connect, ...)
    uint8_t mode;
    /// TX Power
    uint8_t tx_pwr;
    /// Default priority
    uint8_t default_prio;

    /// Internal status
    uint8_t evt_flag;

    /// Flag delete on going
    bool delete_ongoing;
};


/**
 * EM buffer node.
 * 
 * Used for communication with ESP32 BLE peripheral (SoC).
 **/

struct em_buf_node
{
    struct co_list_hdr hdr;
    /// Index of the buffer
    uint16_t idx;
    /// EM buffer pointer
    uint16_t buf_ptr;
};

/**
 * Internal structure used to store incoming packet info.
 **/

typedef struct {
  uint32_t header;
  uint32_t a;
  uint32_t b;
} pkt_hdr_t;

/**
 * EM TX node descriptor.
 * 
 * This structure stores information about a TX PDU.
 **/

struct em_buf_tx_desc
{
    /// tx pointer
    uint16_t txptr;
    /// tx header
    uint16_t txheader;
    /// tx data pointer
    uint16_t txdataptr;
    /// tx data length extension info
    uint16_t txdle;
};


/**
 * EM descriptor node
 * 
 * A descriptor stores information about a TX node (em_buf_node) and some metadata.
 **/

struct em_desc_node
{
    struct co_list_hdr hdr;
    /// Index of the buffer
    uint16_t idx;
    /// EM Pointer of the buffer
    uint16_t buffer_idx;
    ///  Buffer index
    uint16_t buffer_ptr;
    /// Logical Link Identifier
    uint8_t llid;
    /// Data length
    uint8_t length;
};

/**
 * EM main structure.
 **/

struct em_buf_env_tag
{
    /// List of free TX descriptors
    struct co_list tx_desc_free;
    /// List of free TX buffers
    struct co_list tx_buff_free;
    /// Array of TX descriptors (SW tag)
    struct em_desc_node tx_desc_node[0x71];
    /// Array of TX buffer (SW tag)
    struct em_buf_node tx_buff_node[10];
    /// Pointer to TX descriptors
    struct em_buf_tx_desc *tx_desc;
    /// Index of the current RX buffer
    uint8_t rx_current;
};

typedef struct {
    /* Shall we forward this packet ? */
    bool b_forward;
    /* Packet header */
    uint32_t header;
    /* Packet length. */
    uint8_t length;
    /* Packet PDU. */
    uint8_t pdu[256];
} esp_packet_processed_t;

/* Available datarates (1Mbps or 2Mbps). */
typedef enum {
    DATARATE_1M = 0,
    DATARATE_2M = 1
} datarate_t;


/* Available radio modes. */
typedef enum {
    MODE_RX = 0,
    MODE_TX = 1,
    MODE_JAMMER = 2
} radio_mode_t;

/* Structure storing information about the radio configuration. */
typedef struct {
    uint32_t sync_word;
    uint32_t frequency;
    bool swapping;
    datarate_t data_rate;
    mode_t mode;
    bool enabled;
} radio_config_t;

/* Structure representing a received packet and its metadata. */
typedef struct {
  uint8_t *packet;
  size_t total_size;
  int8_t rssi;
  int frequency;
} rx_packet_t;

typedef int (*FBLEHACK_IsrCallback)(int packet_num, uint16_t header, uint8_t *p_pdu, int length);
typedef int (*FBLEHACK_RawCallback)(uint8_t *p_packet, size_t size, int8_t rssi, int frequency);
typedef int (*F_rom_llc_llcp_send)(int conhdl, uint8_t *p_pdu, uint8_t opcode);
typedef void (*FBLEHACK_ProgPacketsCallback)(void);

/* Printf() equivalent required to display debug info when in ISR (callbacks). */
int esp_rom_printf( const char *restrict format, ... );

/* Install hooks. */
void ble_hack_install_hooks(void);
void disconnect_nimble(void);

void ble_hack_rx_control_pdu_handler(FBLEHACK_IsrCallback pfn_control_callback);
void ble_hack_rx_data_pdu_handler(FBLEHACK_IsrCallback pfn_data_callback);
void ble_hack_tx_data_pdu_handler(FBLEHACK_IsrCallback pfn_data_callback);
void ble_hack_tx_prog_packets(FBLEHACK_ProgPacketsCallback pfn_prog_callback);
void ble_hack_raw_packet_handler(FBLEHACK_RawCallback pfn_raw_callback);

void set_packet_length(int packet_num, uint8_t length);

/* Send a data PDU. */
void send_raw_data_pdu(int conhdl, uint8_t llid, uint8_t *p_pdu, int length, bool can_be_freed);


/** Raw packet **/
void set_frequency(uint32_t frequency);
void set_sync_word(uint32_t sync_word);
void set_swapping(bool swapping);
void set_data_rate(datarate_t datarate);
void ble_hack_enable_raw(void);
void ble_hack_disable_raw(void);

#endif /* __INC_ESP32_BLE_HACK_H */