#pragma once

#include <string>
#include <functional>
#include <cstdint>
#include <memory>
#include "IPAddress.hpp"
#include "esp_wifi.h"

namespace espp {
class WifiSta;
class WifiAp;
}

/**
 * @brief Arduino风格的WiFi封装
 * 提供简单易用的station和softAP接口
 *
 * 使用示例 - Station模式:
 * @code
 *   WiFi.begin("my_ssid", "my_password");
 *   while (!WiFi.isConnected()) {
 *       delay(500);
 *   }
 *   printf("IP: %s\n", WiFi.localIP().toString().c_str());
 * @endcode
 *
 * 使用示例 - AP模式:
 * @code
 *   WiFi.softAP("my_ap", "password");
 *   printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
 * @endcode
 */

class WiFiClass {
public:
    using ConnectedCallback = std::function<void(IPAddress ip)>;
    using DisconnectedCallback = std::function<void()>;

    WiFiClass();
    ~WiFiClass();

    // 不允许拷贝
    WiFiClass(const WiFiClass&) = delete;
    WiFiClass& operator=(const WiFiClass&) = delete;

    /**
     * @brief 初始化WiFi为station模式
     * @return *this 支持链式调用
     */
    WiFiClass& begin();

    /**
     * @brief 连接到WiFi (station模式)
     * @param ssid SSID
     * @param password 密码
     * @return *this 支持链式调用
     */
    WiFiClass& begin(const char* ssid, const char* password = nullptr);

    /**
     * @brief 连接到已配置的WiFi (station模式)
     * @return *this 支持链式调用
     */
    WiFiClass& connect();

    /**
     * @brief 连接到WiFi (station模式)
     * @param ssid SSID
     * @param password 密码
     * @return *this 支持链式调用
     */
    WiFiClass& connect(const char* ssid, const char* password = nullptr);

    /**
     * @brief 停止WiFi并断开连接
     * @return *this 支持链式调用
     */
    WiFiClass& end();

    /**
     * @brief 检查是否已连接
     * @return true 已连接
     */
    bool isConnected() const;

    /**
     * @brief 获取RSSI信号强度
     * @return RSSI值 (dBm)
     */
    int8_t rssi() const;

    /**
     * @brief 获取当前连接的SSID
     * @return SSID字符串
     */
    std::string ssid() const;

    /**
     * @brief 获取本地IP地址
     * @return IPAddress
     */
    IPAddress localIP() const;

    /**
     * @brief 获取子网掩码
     * @return IPAddress
     */
    IPAddress subnetMask() const;

    /**
     * @brief 获取网关IP
     * @return IPAddress
     */
    IPAddress gatewayIP() const;

    /**
     * @brief 获取主DNS服务器IP
     * @return IPAddress
     */
    IPAddress dnsIP() const;

    /**
     * @brief 获取备用DNS服务器IP
     * @return IPAddress
     */
    IPAddress dnsIP(int index) const;

    /**
     * @brief 启动SoftAP模式
     * @param ssid AP名称
     * @param password 密码 (至少8位，为空则开放)
     * @param channel WiFi信道
     * @param hidden 是否隐藏SSID
     * @param max_connection 最大连接数
     * @return *this 支持链式调用
     */
    WiFiClass& softAP(const char* ssid, const char* password = nullptr,
                int channel = 1, bool hidden = false, int max_connection = 4);

    /**
     * @brief 获取SoftAP的IP地址
     * @return IPAddress
     */
    IPAddress softAPIP() const;

    /**
     * @brief 获取当前SoftAP连接的客户端数量
     * @return 连接数
     */
    int softAPgetStationNum() const;

    /**
     * @brief 停止SoftAP模式
     * @return *this 支持链式调用
     */
    WiFiClass& softAPend();

    /**
     * @brief 设置WiFi模式
     * @param mode WIFI_MODE_STA / WIFI_MODE_AP / WIFI_MODE_APSTA
     * @return *this 支持链式调用
     */
    WiFiClass& setMode(wifi_mode_t mode);

    /**
     * @brief 获取当前WiFi模式
     * @return wifi_mode_t
     */
    wifi_mode_t getMode() ;

    /**
     * @brief 设置连接成功回调
     * @return *this 支持链式调用
     */
    WiFiClass& onConnected(ConnectedCallback callback);

    /**
     * @brief 设置断开连接回调
     * @return *this 支持链式调用
     */
    WiFiClass& onDisconnected(DisconnectedCallback callback);

    /**
     * @brief 扫描可用网络
     * @param async 是否异步扫描
     * @return 扫描到的网络数量
     */
    int scanNetworks(bool async = false);

    /**
     * @brief 获取扫描结果中某个网络的SSID
     * @param index 索引
     * @return SSID
     */
    std::string getSSID(int index) const;

    /**
     * @brief 获取扫描结果中某个网络的RSSI
     * @param index 索引
     * @return RSSI
     */
    int getRSSI(int index) const;

    /**
     * @brief 获取扫描结果中某个网络是否加密
     * @param index 索引
     * @return true 加密
     */
    bool getEncrypted(int index) const;

    /**
     * @brief 获取扫描结果中某个网络的信道
     * @param index 索引
     * @return 信道
     */
    int getChannel(int index) const;

    /**
     * @brief 获取底层esp_netif句柄 (station)
     */
    esp_netif_t* getStaNetif();

    /**
     * @brief 获取底层esp_netif句柄 (ap)
     */
    esp_netif_t* getApNetif();

    /**
     * @brief 获取当前 hostname
     * @return hostname 字符串
     */
    std::string getHostname() const;

    /**
     * @brief 设置 hostname
     * @param hostname 新的 hostname
     * @return true 成功, false 失败
     */
    WiFiClass& setHostname(const char *hostname);


   
private:
    void handle_ip(ip_event_got_ip_t *ip_evt);
    void handle_disconnect();

    // espp WiFi 实例 - 支持 STA 和 AP 共存
    std::unique_ptr<espp::WifiSta> sta_;
    std::unique_ptr<espp::WifiAp> ap_;

    // WiFi模式
    wifi_mode_t m_mode{WIFI_MODE_NULL};

    // 连接状态
    bool m_connected{false};

    // 扫描结果缓存
    wifi_ap_record_t *m_scan_results{nullptr};
    uint16_t m_scan_count{0};

    // 回调
    ConnectedCallback m_connected_callback;
    DisconnectedCallback m_disconnected_callback;
};

// 全局单例，就像Arduino一样
extern WiFiClass WiFi;
