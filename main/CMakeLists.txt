# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES twatch-lib)
set(COMPONENT_PRIV_REQUIRES nvs_flash bt)

set(
 COMPONENT_SRCS 
 "main.c"
 "menu.c"
 "img/digits_8bpp.c"
 "img/digits_small_8bpp.c"
 "img/ticks_8bpp.c"
 "ui/tile-clock.c"
 "ui/tile-settings.c"
 "ui/timeset.c"
 "ui/dateset.c"
 "ui/battery.c"
)

if (CONFIG_INCLUDE_IR)
  list(APPEND COMPONENT_SRCS
    "ui/tile-tvbgone.c"
  )
endif()

if (CONFIG_INCLUDE_BLE)
    list(APPEND COMPONENT_SRCS
        "ble/ble.c"
        "ble/ble_hack.c"
        "img/ble_icons_8bpp.c"
        "img/ble_icons_big_8bpp.c"
        "ui/tile-blescan.c"
        "ui/tile-bleinfo.c"
    )
endif()

if (CONFIG_INCLUDE_WIFI)
  list(APPEND COMPONENT_SRCS
    "wifi/wifi.c"
    "wifi/wifi_aplist.c"
    "wifi/wifiscan.c"
    "wifi/dissect.c"
  )

  if (CONFIG_WIFI_SCANNER)
    list( APPEND COMPONENT_SRCS
    "ui/tile-scanner.c"
    "ui/tile-apinfo.c"
    )
  endif()

  if (CONFIG_WIFI_CHANNELS)
    list( APPEND COMPONENT_SRCS "ui/tile-channels.c")
  endif()

  if (CONFIG_WIFI_ROGUEAP)
    list( APPEND COMPONENT_SRCS "ui/tile-rogueap.c")
  endif()

  if (CONFIG_WIFI_DEAUTH)
    list( APPEND COMPONENT_SRCS "ui/tile-deauth.c")
  endif()
endif()


register_component()
