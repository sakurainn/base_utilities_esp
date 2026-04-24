# Base source files
set(SOURCES
    src/IPAddress.cpp
    src/WiFi.cpp
    src/BaseUtilities.cpp
    deps/wifi/src/wifi_ap.cpp
    deps/wifi/src/wifi_sta.cpp
    deps/simple_uuid/simple_uuid.c
    deps/stb/stb_ds.c
    deps/flashdb/src/fdb_file.c
    deps/flashdb/src/fdb_kvdb.c
    deps/flashdb/src/fdb_tsdb.c
    deps/flashdb/src/fdb_utils.c
    deps/flashdb/src/fdb.c
    deps/ticker/Ticker.cpp
    deps/esp_event_cxx/esp_event_cxx.cpp
    deps/esp_event_cxx/esp_event_api.cpp
    deps/esp_event_cxx/esp_exception.cpp
    deps/preferences/Preferences.cpp
    src/UUID.cpp
    src/Timer.cpp
)

# Base required components
set(REQUIRES
    driver
    nvs_flash
    freertos
    esp_wifi
    esp_event
    esp_timer
    esp-tls
    esp_websocket_client

    log
    json
)


idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include" "deps"  "deps/esp_event_cxx" "deps/magic_enum"
    REQUIRES ${REQUIRES}
    PRIV_INCLUDE_DIRS "deps/wifi/include" "deps/simple_uuid" "deps/" "deps/flashdb/include" "deps/ticker"
    
        "deps/preferences"
)
