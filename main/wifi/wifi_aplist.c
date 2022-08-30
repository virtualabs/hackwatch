#include <string.h>
#include "esp_log.h"
#include "wifi_aplist.h"

#define TAG "wifi_aplist"


/**
 * wifi_aplist_init()
 * 
 * @brief: Initialize a list of AP info
 * @param p_list: pointer to a `wifi_ap_t` structure
 **/

void wifi_aplist_init(wifi_aplist_t *p_list)
{
  /* Initialize our list. */
  p_list->count = 0;
  p_list->p_first = NULL;

  ESP_LOGD(TAG, "list initialized");
}


/**
 * wifi_aplist_clean()
 **/

void wifi_aplist_clean(wifi_aplist_t *p_list)
{
  wifi_ap_t *p_item;
  wifi_ap_t *p_previous;
  
  ESP_LOGD(TAG, "Clean list");

  p_previous = NULL;
  p_item = p_list->p_first;
  while (p_item != NULL)
  {
    /* Decrease freshness. */
    p_item->freshness--;
    if (p_item->freshness < WIFI_AP_FRESHNESS_OLD)
    {
      ESP_LOGD(TAG, "%s is not so fresh, removing it", p_item->essid);

      /* Remove item */
      if (p_previous == NULL)
      {
        /* Update our pointer to the first item and free this item. */
        p_list->p_first = p_item->p_next;
        free(p_item);

        /* Go on with the next item. */
        p_item = p_list->p_first;
      }
      else
      {
        /* Skip the item to be removed. */
        p_previous->p_next = p_item->p_next;

        /* Free the item (remove). */
        free(p_item);

        /* Go on with next item. */
        p_item = p_previous->p_next;
      }

      /* Decrease number of items. */
      p_list->count--;
    }
    else
    {
      ESP_LOGD(TAG, "%s is fresh enough", p_item->essid);

      /* Keep previous up-to-date. */
      p_previous = p_item;

      /* Go on with next item. */
      p_item = p_item->p_next;
    }
  }
}


/**
 * wifi_aplist_alloc_item()
 * 
 * @brief: Allocate and initialize a list item.
 * @param ap: pointer to a `wifi_ap_record` structure.
 * @return: pointer to the newly allocated item or NULL on error.
 **/

wifi_ap_t *wifi_aplist_alloc_item(wifi_ap_record_t *ap)
{
  int i;
  wifi_ap_t *p_item;

  ESP_LOGD(TAG, "allocate wifi aplist item");
  p_item = (wifi_ap_t *)malloc(sizeof(wifi_ap_t));
  if (p_item != NULL)
  {
    ESP_LOGD(TAG, "wifi ap item allocated");

    /* Initialize item. */
    for (i=0; i<6; i++)
      p_item->bssid[i] = ap->bssid[i];

    strncpy((char*)p_item->essid, (char*)ap->ssid, 33);
    p_item->rssi = ap->rssi;
    p_item->channel = ap->primary;
    p_item->auth_mode = ap->authmode;
    p_item->pw_cipher = ap->pairwise_cipher;
    p_item->gp_cipher = ap->group_cipher;
    p_item->freshness = WIFI_AP_FRESHNESS_MAX;
    
    /* No next item. */
    p_item->p_next = NULL;

    /* Return item. */
    return p_item;
  }
  else
  {
    ESP_LOGE(TAG, "memory error: could not allocate item");
    
    /* Could not allocate, return NULL. */
    return NULL;
  }
}


void wifi_aplist_add(wifi_aplist_t *p_list, wifi_ap_record_t *ap)
{
  wifi_ap_t *p_item, *p_anchor;

  if (p_list->count == 0)
  {
    ESP_LOGD(TAG, "list is empty, allocating item");
    p_item = wifi_aplist_alloc_item(ap);
    if (p_item != NULL)
    {
      ESP_LOGD(TAG, "saving item to list");
      /* Save item. */
      p_list->p_first = p_item;
      p_list->count++;
    }
  }
  else if (p_list->p_first != NULL)
  {
    ESP_LOGD(TAG, "list not empty, looking for an existing ap ...");

    /* First, we walk the list to avoid a potential double. */
    p_item = p_list->p_first;
    while (p_item != NULL)
    {
      /* Do we already know this AP ? */
      if (!memcmp(p_item->bssid, ap->bssid, 6))
      {
        ESP_LOGD(TAG, "found this ap, updating data ...");
        /* Yep, update its info. */
        memcpy(p_item->essid, ap->ssid, 33);
        p_item->rssi = ap->rssi;
        p_item->channel = ap->primary;
        p_item->auth_mode = ap->authmode;
        p_item->pw_cipher = ap->pairwise_cipher;
        p_item->gp_cipher = ap->group_cipher;
        p_item->freshness = WIFI_AP_FRESHNESS_MAX;

        /* Clean list. */
        wifi_aplist_clean(p_list);

        /* Item added, exit. */
        return;
      }
      else if (p_item->p_next == NULL)
      {
        break;
      }
      else
        /* Go to the next item. */
        p_item = p_item->p_next;
    }

    ESP_LOGD(TAG, "ap not found, inserting info into our list");

    /* Add AP if there is some space left. */
    if (p_list->count < WIFI_APLIST_MAX_NUMBER)
    {
      /* Item is appended at the end of the list. */
      p_anchor = p_item;
      p_item = wifi_aplist_alloc_item(ap);
      if (p_item != NULL)
      {
        /* Link item. */
        p_anchor->p_next = p_item;
        p_list->count++;

        /* Clean list. */
        wifi_aplist_clean(p_list);
      }
    }
  }
}

/**
 * wifi_aplist_enum_first()
 * 
 * @brief: Retrieve the first item of an AP list.
 * @param p_list: pointer to a `wifi_aplist_t` structure
 * @return: a pointer to the first wifi_ap_t structure of the list, NULL otherwise
 **/

wifi_ap_t *wifi_aplist_enum_first(wifi_aplist_t *p_list)
{
  if (p_list->p_first != NULL)
    return p_list->p_first;
  else
    return NULL;
}


/**
 * wifi_aplist_enum_next()
 * 
 * @brief: Retrieve the next item of an AP list.
 * @param p_list: pointer to the previous `wifi_ap_t` structure
 * @return: a pointer to the first wifi_ap_t structure of the list, NULL otherwise
 **/

wifi_ap_t *wifi_aplist_enum_next(wifi_ap_t *ap)
{
  if (ap->p_next != NULL)
    return ap->p_next;
  else
    return NULL;
}