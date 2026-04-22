#include "WiFi.hpp"

#include <arpa/inet.h>
#include <esp_netif_ip_addr.h>

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include "IPAddress.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "wifi_ap.hpp"
#include "wifi_sta.hpp"


// 全局单例
WiFiClass WiFi;

WiFiClass::WiFiClass() {}

WiFiClass::~WiFiClass() {
  end();
  if (m_scan_results) {
    free(m_scan_results);
  }
}

WiFiClass& WiFiClass::begin() {
  // ESP netif and event loop must be initialized by application
  // WifiSta constructor will handle the rest
  m_mode = WIFI_MODE_STA;
  return *this;
}

WiFiClass& WiFiClass::begin(const char* ssid, const char* password) {
  begin();

  // 创建STA配置
  espp::WifiSta::Config config;
  config.auto_connect = false;
  config.ssid = ssid;
  config.num_connect_retries = SIZE_MAX;
  if (password) {
    config.password = password;
  }

  // 设置回调
  config.on_got_ip = [this](ip_event_got_ip_t* evt) { this->handle_ip(evt); };
  config.on_disconnected = [this]() { this->handle_disconnect(); };

  // 创建WifiSta实例
  sta_ = std::make_unique<espp::WifiSta>(config);

  setMode(WIFI_MODE_STA);
  return *this;
}

WiFiClass& WiFiClass::connect() {
  // WifiSta constructor already starts it
  if (sta_ && !sta_->is_connected()) {
    sta_->connect();
  }
  return *this;
}

WiFiClass& WiFiClass::connect(const char* ssid, const char* password) {
  begin(ssid, password);
  connect();
  return *this;
}

WiFiClass& WiFiClass::end() {
  if (sta_) {
    sta_->stop();
    sta_.reset();
  }
  if (ap_) {
    ap_->stop();
    ap_.reset();
  }
  m_connected = false;
  m_mode = WIFI_MODE_NULL;
  if (m_scan_results) {
    free(m_scan_results);
    m_scan_results = nullptr;
    m_scan_count = 0;
  }
  return *this;
}

bool WiFiClass::isConnected() const {
  if (sta_) {
    return sta_->is_connected();
  }
  return m_connected;
}

int8_t WiFiClass::rssi() const {
  if (!isConnected() || !sta_) {
    return 0;
  }
  wifi_ap_record_t info;
  if (esp_wifi_sta_get_ap_info(&info) != ESP_OK) {
    return 0;
  }
  return info.rssi;
}

std::string WiFiClass::ssid() const {
  if (sta_) {
    return sta_->get_ssid();
  }
  return "";
}

IPAddress WiFiClass::localIP() const {
  if (!isConnected() || !sta_) {
    return IPAddress(0, 0, 0, 0);
  }
  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(sta_->get_netif(), &ip_info) != ESP_OK) {
    return IPAddress(0, 0, 0, 0);
  }
  return IPAddress(ntohl(ip_info.ip.addr));
}

IPAddress WiFiClass::subnetMask() const {
  if (!sta_) {
    return IPAddress(0, 0, 0, 0);
  }
  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(sta_->get_netif(), &ip_info) != ESP_OK) {
    return IPAddress(0, 0, 0, 0);
  }
  return IPAddress(ntohl(ip_info.netmask.addr));
}

IPAddress WiFiClass::gatewayIP() const {
  if (!sta_) {
    return IPAddress(0, 0, 0, 0);
  }
  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(sta_->get_netif(), &ip_info) != ESP_OK) {
    return IPAddress(0, 0, 0, 0);
  }
  return IPAddress(ntohl(ip_info.gw.addr));
}

IPAddress WiFiClass::dnsIP() const {
  if (!sta_) {
    return IPAddress(0, 0, 0, 0);
  }
  esp_netif_dns_info_t dns;
  if (esp_netif_get_dns_info(sta_->get_netif(), ESP_NETIF_DNS_MAIN, &dns) !=
      ESP_OK) {
    return IPAddress(0, 0, 0, 0);
  }
  return IPAddress(ntohl(dns.ip.u_addr.ip4.addr));
}

IPAddress WiFiClass::dnsIP(int index) const {
  if (!sta_) {
    return IPAddress(0, 0, 0, 0);
  }
  esp_netif_dns_type_t dns_type;
  if (index == 0) {
    dns_type = ESP_NETIF_DNS_MAIN;
  } else {
    dns_type = ESP_NETIF_DNS_BACKUP;
  }
  esp_netif_dns_info_t dns;
  if (esp_netif_get_dns_info(sta_->get_netif(), dns_type, &dns) !=
      ESP_OK) {
    return IPAddress(0, 0, 0, 0);
  }
  return IPAddress(ntohl(dns.ip.u_addr.ip4.addr));
}

WiFiClass& WiFiClass::softAP(const char* ssid, const char* password,
                             int channel, bool hidden, int max_connection) {
  (void)hidden;

  // 创建AP配置
  espp::WifiAp::Config config;

  config.ssid = ssid;
  if (password && strlen(password) >= 8) {
    config.password = password;
  }
  config.channel = channel;
  config.max_number_of_stations = max_connection;

  // 创建WifiAp实例
  ap_ = std::make_unique<espp::WifiAp>(config);

  // 如果已经有STA，设置为APSTA模式
  if (sta_) {
    setMode(WIFI_MODE_APSTA);
  } else {
    setMode(WIFI_MODE_AP);
  }

  return *this;
}

IPAddress WiFiClass::softAPIP() const {
  if (!ap_) {
    return IPAddress(0, 0, 0, 0);
  }
  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(ap_->get_netif(), &ip_info) != ESP_OK) {
    return IPAddress(0, 0, 0, 0);
  }
  return IPAddress(ntohl(ip_info.ip.addr));
}

int WiFiClass::softAPgetStationNum() const {
  if (!ap_) {
    return 0;
  }
  return ap_->get_connected_stations().size();
}

WiFiClass& WiFiClass::softAPend() {
  if (ap_) {
    ap_->stop();
    ap_.reset();
  }
  if (sta_) {
    setMode(WIFI_MODE_STA);
  } else {
    setMode(WIFI_MODE_NULL);
  }
  return *this;
}

WiFiClass& WiFiClass::setMode(wifi_mode_t mode) {
  m_mode = mode;
  if (esp_wifi_get_mode(NULL) == ESP_OK) {
    esp_wifi_set_mode(mode);
  }
  return *this;
}

wifi_mode_t WiFiClass::getMode() {
  if (esp_wifi_get_mode(&m_mode) != ESP_OK) {
    return WIFI_MODE_NULL;
  }
  return m_mode;
}

int WiFiClass::scanNetworks(bool async) {
  if (!sta_) {
    return 0;
  }

  // 释放之前的结果
  if (m_scan_results) {
    free(m_scan_results);
    m_scan_results = nullptr;
    m_scan_count = 0;
  }

  // 使用WifiSta的扫描功能
  auto results = sta_->scan(20, false);
  m_scan_count = results.size();
  if (m_scan_count == 0) {
    return 0;
  }

  m_scan_results =
      (wifi_ap_record_t*)malloc(m_scan_count * sizeof(wifi_ap_record_t));
  if (!m_scan_results) {
    return 0;
  }

  // 复制结果
  for (uint16_t i = 0; i < m_scan_count; i++) {
    m_scan_results[i] = results[i];
  }

  return m_scan_count;
}

std::string WiFiClass::getSSID(int index) const {
  if (index < 0 || index >= (int)m_scan_count || !m_scan_results) {
    return "";
  }
  return std::string((const char*)m_scan_results[index].ssid);
}

int WiFiClass::getRSSI(int index) const {
  if (index < 0 || index >= (int)m_scan_count || !m_scan_results) {
    return 0;
  }
  return m_scan_results[index].rssi;
}

bool WiFiClass::getEncrypted(int index) const {
  if (index < 0 || index >= (int)m_scan_count || !m_scan_results) {
    return false;
  }
  return m_scan_results[index].authmode != WIFI_AUTH_OPEN;
}

int WiFiClass::getChannel(int index) const {
  if (index < 0 || index >= (int)m_scan_count || !m_scan_results) {
    return 0;
  }
  return m_scan_results[index].primary;
}

esp_netif_t* WiFiClass::getStaNetif() {
  if (sta_) {
    return sta_->get_netif();
  }
  return nullptr;
}

esp_netif_t* WiFiClass::getApNetif() {
  if (ap_) {
    return ap_->get_netif();
  }
  return nullptr;
}

std::string WiFiClass::getHostname() const {
  if (sta_) {
    return sta_->get_hostname();
  }
  if (ap_) {
    return ap_->get_hostname();
  }
  return "";
}

WiFiClass& WiFiClass::setHostname(const char *hostname) {
  bool result = false;
  if (sta_) {
    result |= sta_->set_hostname(hostname);
  }
  if (ap_) {
    result |= ap_->set_hostname(hostname);
  }
  return *this;
}

void WiFiClass::handle_ip(ip_event_got_ip_t* ip_evt) {
  m_connected = true;
  IPAddress ip(ntohl(ip_evt->ip_info.ip.addr));
  if (m_connected_callback) {
    m_connected_callback(ip);
  }
}

void WiFiClass::handle_disconnect() {
  m_connected = false;
  if (m_disconnected_callback) {
    m_disconnected_callback();
  }
}

WiFiClass& WiFiClass::onConnected(ConnectedCallback callback) {
  m_connected_callback = std::move(callback);
  return *this;
}

WiFiClass& WiFiClass::onDisconnected(DisconnectedCallback callback) {
  m_disconnected_callback = std::move(callback);
  return *this;
}
