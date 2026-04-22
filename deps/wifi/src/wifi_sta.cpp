#include "wifi_sta.hpp"
#include "wifi_internal.hpp"

namespace espp {

void WifiSta::init(const Config &config) {
  // Store the configuration for later recovery if needed
  config_ = config;

  if (netif_ == nullptr) {
    ESP_LOGI(TAG, "Creating network interface and ensuring WiFi stack is initialized");
    auto &wifi = Wifi::get();
    if (!wifi.init()) {
      ESP_LOGE(TAG, "Could not initialize WiFi stack");
      return;
    }
    netif_ = wifi.get_sta_netif();
  }

  if (!register_event_handlers()) {
    ESP_LOGE(TAG, "Could not register event handlers");
    return;
  }

  if (!reconfigure(config)) {
    ESP_LOGE(TAG, "Could not configure WiFi STA");
    return;
  }

  if (!start()) {
    ESP_LOGE(TAG, "Could not start WiFi STA");
    return;
  }

  ESP_LOGI(TAG, "WiFi STA started, SSID: '%s'", config.ssid.c_str());
}

} // namespace espp
