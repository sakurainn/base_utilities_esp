#include "wifi_ap.hpp"
#include "wifi_internal.hpp"

namespace espp {

void WifiAp::init(const Config &config) {
  if (netif_ == nullptr) {
    ESP_LOGI(TAG, "Creating network interface and ensuring WiFi stack is initialized");
    auto &wifi = Wifi::get();
    if (!wifi.init()) {
      ESP_LOGE(TAG, "Could not initialize WiFi stack");
      return;
    }
    netif_ = wifi.get_ap_netif();
  }

  if (!register_event_handlers()) {
    ESP_LOGE(TAG, "Could not register event handlers");
    return;
  }

  if (!reconfigure(config)) {
    ESP_LOGE(TAG, "Could not configure WiFi AP");
    return;
  }

  if (!start()) {
    ESP_LOGE(TAG, "Could not start WiFi AP");
    return;
  }

  ESP_LOGI(TAG, "WiFi AP started, SSID: '%s', Password: '%s'", config.ssid.c_str(), config.password.c_str());
}

} // namespace espp
