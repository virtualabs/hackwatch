#ifndef __INC_WIFI_DISSECT_H
#define __INC_WIFI_DISSECT_H

#include <string.h>
#include "esp_err.h"

#define   PKT_TYPE_MANAGEMENT   0x00
#define   TLV_ELEMENT_ESSID     0x00

/**
 * Enums
 **/

typedef enum {
  PKT_ASSOC_REQ = 0,
  PKT_ASSOC_RSP,
  PKT_REASSOC_REQ,
  PKT_REASSOC_RSP,
  PKT_PROBE_REQ,
  PKT_PROBE_RSP,
  PKT_BEACON = 8,
  PKT_ATIM,
  PKT_DISASSOC,
  PKT_AUTH,
  PKT_DEAUTH
} wifi_pkt_subtype_t;

/**
 * Structures
 **/

__attribute__((packed))
typedef struct {
  uint8_t element_id;
  uint8_t length;
  void *p_data;
} wifi_tlv_t;

__attribute__((packed))
typedef struct {
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
} wifi_mac_header_t;


typedef struct {
  wifi_mac_header_t header;
  char essid[32];
} wifi_probe_req_t;

typedef struct {
  wifi_mac_header_t header;
  char essid[32];
} wifi_probe_rsp_t;

/* Exported functions. */
esp_err_t wifi_pkt_get_type(void *buffer, wifi_pkt_subtype_t *subtype);
esp_err_t wifi_pkt_parse_probe_req(void *buffer, wifi_probe_req_t *p_probe_req);
esp_err_t wifi_pkt_parse_probe_rsp(void *buffer, wifi_probe_rsp_t *p_probe_rsp);



#endif /* __INC_WIFI_DISSECT_H */