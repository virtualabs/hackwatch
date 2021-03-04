#include "dissect.h"


/**
 * tlv_parse()
 * 
 * @brief: Parse TLV elements
 * @param buffer: pointer to a wifi packet as provided by ESP32
 * @param p_tlv_element: pointer to a `wifi_tlv_t` structure that will be filled by tlv_parse()
 * @return: Number of bytes parsed, <0 on error (not handled for now)
 **/

int tlv_parse(void *buffer, wifi_tlv_t *p_tlv_element)
{
  wifi_tlv_t *p_item = (wifi_tlv_t *)buffer;

  /* Write element id in destination structure. */
  p_tlv_element->element_id = p_item->element_id;

  /* TODO: ensure element_id exists. */

  /* Write length in destination structure. */
  p_tlv_element->length = p_item->length;

  /* Copy data into destination structure. */
  p_tlv_element->p_data = (void *)((uint8_t *)buffer + 2);

  /* Succes, return number of bytes parsed. */
  return (p_tlv_element->length + 2);
}

esp_err_t wifi_pkt_get_type(void *buffer, wifi_pkt_subtype_t *subtype)
{
  uint8_t pkt_subtype, pkt_type;
  wifi_mac_header_t *p_mac_header = (wifi_mac_header_t *)buffer;

  /* Parse packet type and subtype. */
  pkt_type = (p_mac_header->frame_ctrl >> 2) & 0x03;
  pkt_subtype = (p_mac_header->frame_ctrl >> 4) & 0x0F;

  if (pkt_type == PKT_TYPE_MANAGEMENT)
  {
    /* Return packet subtype. */
    *subtype = (wifi_pkt_subtype_t) pkt_subtype;
    return ESP_OK;
  }
  else
  {
    /* Unable to dissect non-management frames. */
    return ESP_FAIL;
  }
}


/**
 * wifi_pkt_parse_probe_req()
 * 
 * @brief: Parse a probe request, extract the ESSID field.
 * @param buffer: pointer ot a wifi packet as provided by ESP32.
 * @param p_probe_req: pointer to a `wifi_probe_req` structure that will be filled by this function.
 * @return: ESP_OK on success, ESP_FAIL on error.
 **/

esp_err_t wifi_pkt_parse_probe_req(void *buffer, wifi_probe_req_t *p_probe_req)
{
  wifi_tlv_t tlv_essid;
  wifi_pkt_subtype_t subtype;
  wifi_mac_header_t *p_mac_header = (wifi_mac_header_t *)buffer;

  if (wifi_pkt_get_type(buffer, &subtype) == ESP_OK)
  {
    /* Check subtype. */
    if (subtype == PKT_PROBE_REQ)
    {
      /* Copy MAC header into our probe request structure. */
      memcpy(p_probe_req, p_mac_header, sizeof(wifi_mac_header_t));

      /* Extract ESSID */
      if (tlv_parse((void *)((uint8_t *)p_mac_header + sizeof(wifi_mac_header_t)), &tlv_essid) > 0)
      {
       
        /* Ensure element ID == ESSID */
        if (tlv_essid.element_id == TLV_ELEMENT_ESSID)
        {
          /* No more than 32 bytes. */
          if (tlv_essid.length >= 32)
            tlv_essid.length = 31;

          /* Save ESSID to our structure. */
          memcpy(p_probe_req->essid, tlv_essid.p_data, tlv_essid.length + 1);
          p_probe_req->essid[tlv_essid.length] = 0;

          /* Success. */
          return ESP_OK;
        }
        else
        {
          /* Not an ESSID element. */
          return ESP_FAIL;
        }
      }
      else
      {
        /* Failed parsing ESSID. */
        return ESP_FAIL;
      }

    }
    else
    {
      /* Not a probe request. */
      return ESP_FAIL;
    }

  }
  else
  {
    /* Not a valid packet. */
    return ESP_FAIL;
  }
}


/**
 * wifi_pkt_parse_probe_rsp()
 * 
 * @brief: Parse a probe response, extract the ESSID field.
 * @param buffer: pointer ot a wifi packet as provided by ESP32.
 * @param p_probe_req: pointer to a `wifi_probe_rsp` structure that will be filled by this function.
 * @return: ESP_OK on success, ESP_FAIL on error.
 **/

esp_err_t wifi_pkt_parse_probe_rsp(void *buffer, wifi_probe_rsp_t *p_probe_rsp)
{
  wifi_tlv_t tlv_essid;
  wifi_pkt_subtype_t subtype;
  wifi_mac_header_t *p_mac_header = (wifi_mac_header_t *)buffer;

  if (wifi_pkt_get_type(buffer, &subtype) == ESP_OK)
  {
    /* Check subtype. */
    if (subtype == PKT_PROBE_RSP)
    {
      /* Copy MAC header into our probe request structure. */
      memcpy(p_probe_rsp, p_mac_header, sizeof(wifi_mac_header_t));

      /* Extract ESSID */
      if (tlv_parse((void *)((uint8_t *)p_mac_header + sizeof(wifi_mac_header_t) + 12), &tlv_essid) > 0)
      {
       
        /* Ensure element ID == ESSID */
        if (tlv_essid.element_id == TLV_ELEMENT_ESSID)
        {
          /* No more than 32 bytes. */
          if (tlv_essid.length >= 32)
            tlv_essid.length = 31;

          /* Save ESSID to our structure. */
          memcpy(p_probe_rsp->essid, tlv_essid.p_data, tlv_essid.length + 1);
          p_probe_rsp->essid[tlv_essid.length] = 0;

          /* Success. */
          return ESP_OK;
        }
        else
        {
          /* Not an ESSID element. */
          return ESP_FAIL;
        }
      }
      else
      {
        /* Failed parsing ESSID. */
        return ESP_FAIL;
      }

    }
    else
    {
      /* Not a probe response. */
      return ESP_FAIL;
    }
  }
  else
  {
    /* Not a valid packet. */
    return ESP_FAIL;
  }
}