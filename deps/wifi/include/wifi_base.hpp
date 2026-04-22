#pragma once

#include <string>

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include <format>
namespace espp {

/**
 * @brief Base class for WiFi interfaces (AP and STA)
 * @details Provides common functionality shared between WiFi Access Point
 *          and WiFi Station implementations.
 */
class WifiBase {
public:
  /**
   * @brief Virtual destructor
   */
  virtual ~WifiBase() = default;

  /**
   * @brief Get the hostname of the WiFi interface
   * @return Current hostname as a string
   */
  std::string get_hostname() {
    const char *hostname;
    esp_err_t err = esp_netif_get_hostname(netif_, &hostname);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Could not get hostname: %s", esp_err_to_name(err));
      return "";
    }
    return std::string(hostname);
  }

  /**
   * @brief Set the hostname of the WiFi interface
   * @param hostname New hostname to set
   * @return True if successful, false otherwise
   */
  virtual bool set_hostname(const std::string &hostname) {
    esp_err_t err = esp_netif_set_hostname(netif_, hostname.c_str());
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Could not set hostname: %s", esp_err_to_name(err));
      return false;
    }
    ESP_LOGI(TAG, "Hostname set to '%s'", hostname.c_str());
    return true;
  }

  /**
   * @brief Get the MAC address as a string
   * @return MAC address in format "XX:XX:XX:XX:XX:XX"
   */
  virtual std::string get_mac_address_string() = 0;

  /**
   * @brief Get the IP address as a string
   * @return IP address as a string
   */
  virtual std::string get_ip_address() {
    if (netif_ == nullptr) {
      return "";
    }
    esp_netif_ip_info_t ip_info;
    ESP_LOGI(TAG, "Getting IP address...");
    esp_err_t err = esp_netif_get_ip_info(netif_, &ip_info);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Could not get IP address: %s", esp_err_to_name(err));
      return "";
    }
    return std::format("{}.{}.{}.{}", IP2STR(&ip_info.ip));
  }

  /**
   * @brief Check if the interface is connected/active
   * @return True if connected/active, false otherwise
   */
  virtual bool is_connected() const = 0;

  /**
   * @brief Get the SSID
   * @return Current SSID as a string
   */
  virtual std::string get_ssid() = 0;

  /**
   * @brief Start the WiFi interface
   * @return True if successful, false otherwise
   */
  virtual bool start() = 0;

  /**
   * @brief Stop the WiFi interface
   * @return True if successful, false otherwise
   */
  virtual bool stop() = 0;

  /**
   * @brief Get the network interface pointer
   * @return Pointer to the esp_netif_t structure
   */
  esp_netif_t *get_netif() const { return netif_; }

  /**
   * @brief Check if WiFi subsystem has been initialized
   * @return True if WiFi is initialized, false otherwise
   * @note This is a static method that checks global WiFi state
   */
  static bool is_initialized() {
    wifi_mode_t mode;
    bool success = esp_wifi_get_mode(&mode) == ESP_OK;
    bool valid = mode != WIFI_MODE_NULL;
    return success && valid;
  }

protected:
  const char* TAG;

  /**
   * @brief Constructor for WifiBase
   * @param tag Logger tag
   */
  explicit WifiBase(const char *tag)
      : TAG(tag) {}

  /**
   * @brief Constructor for WifiBase
   * @param tag Logger tag
   * @param netif Pointer to the network interface
   */
  explicit WifiBase(const char *tag, esp_netif_t *netif)
      : TAG(tag)
      , netif_(netif) {}

  esp_netif_t *netif_{nullptr};
};

} // namespace espp
