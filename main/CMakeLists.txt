# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES twatch-lib)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c wifi/wifi.c wifi/wifi_aplist.c wifi/wifiscan.c wifi/dissect.c ui/tile-scanner.c ui/tile-apinfo.c ui/tile-clock.c")

register_component()
