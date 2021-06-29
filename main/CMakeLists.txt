# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES twatch-lib)
set(COMPONENT_PRIV_REQUIRES )

set(
 COMPONENT_SRCS 
 "main.c"
 "img/digits.c"
 "img/digits_small.c"
 "img/ticks.c"
 "ir/tvbgone.c"
 "wifi/wifi.c"
 "wifi/wifi_aplist.c"
 "wifi/wifiscan.c"
 "wifi/dissect.c"
 "ui/tile-scanner.c"
 "ui/tile-apinfo.c"
 "ui/tile-clock.c"
 "ui/tile-settings.c"
 "ui/tile-channels.c"
 "ui/tile-rogueap.c"
 "ui/tile-ir-shutdown.c"
 "ui/timeset.c"
)


register_component()
