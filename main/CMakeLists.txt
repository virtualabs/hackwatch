set(
 SOURCES
 "main.c" 
 "menu.c"
 "img/digits.c"
 "img/digits_small.c"
 "img/ticks.c"
 "ui/tile-clock.c"
 "ui/tile-settings.c"
 "ui/timeset.c"
 "ui/dateset.c"
 "ui/battery.c"
)

if (CONFIG_INCLUDE_IR)
  list(APPEND SOURCES
    "ui/tile-tvbgone.c"
  )
endif()

if (CONFIG_INCLUDE_BLE)
    list(APPEND SOURCES
        "ble/ble.c"
        "ble/ble_hack.c"
        "img/ble_icons.c"
        "img/ble_icons_big.c"
        "ui/tile-blescan.c"
        "ui/tile-bleinfo.c"
    )
endif()

if (CONFIG_INCLUDE_WIFI)
  list(APPEND SOURCES
    "wifi/wifi.c"
    "wifi/wifi_aplist.c"
    "wifi/wifiscan.c"
    "wifi/dissect.c"
  )

  if (CONFIG_WIFI_SCANNER)
    list( APPEND SOURCES
    "ui/tile-scanner.c"
    "ui/tile-apinfo.c"
    )
  endif()

  if (CONFIG_WIFI_CHANNELS)
    list( APPEND SOURCES "ui/tile-channels.c")
  endif()

  if (CONFIG_WIFI_ROGUEAP)
    list( APPEND SOURCES "ui/tile-rogueap.c")
  endif()
endif()

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
)
