# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES twatch-lib)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c wifi/wifi.c wifi/wifi_aplist.c wifi/wifiscan.c")

register_component()
