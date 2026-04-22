#pragma once

#include <esp_crt_bundle.h>
#include <esp_websocket_client.h>
#include <esp_wifi.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include<esp_log.h>
#include "DelayExec.hpp"

/**
 * @brief 基于esp_websocket_client的C++封装
 * 支持:
 *  - 连接/断开WebSocket服务器
 *  - 发送文本/二进制数据
 *  - 接收数据回调
 *  - 连接状态变化回调
 *  - 自动重连机制
 *
 * 使用示例:
 * @code
 *   WebsocketClient client;
 *
 *   // 设置回调
 *   client.on_connected([](){
 *       printf("websocket connected!\n");
 *   });
 *
 *   client.on_disconnected([](){
 *       printf("websocket disconnected!\n");
 *   });
 *
 *   client.on_message([](const char* data, size_t len){
 *       printf("received: %.*s\n", (int)len, data);
 *   });
 *
 *   // 连接到服务器
 *   client.connect("wss://echo.websocket.events");
 *
 *   // 发送数据
 *   client.send_text("Hello World!");
 *
 *   // 断开连接
 *   client.disconnect();
 * @endcode
 */
class WebsocketClient {
 public:
  using ConnectedCallback = std::function<void()>;
  using DisconnectedCallback = std::function<void()>;
  using MessageCallback = std::function<void(ws_transport_opcodes_t opcode,
                                             const char* data, size_t len)>;
  using ErrorCallback =
      std::function<void(esp_websocket_error_codes_t error_code)>;

  /**
   * @brief 构造函数
   */
  WebsocketClient()
      : m_client(nullptr),
        m_connected(false),
        m_auto_reconnect(false),
        m_reconnect_delay_ms(5000) {}

  /**
   * @brief 析构函数，自动断开连接
   */
  ~WebsocketClient() { disconnect(); }

  // 不允许拷贝
  WebsocketClient(const WebsocketClient&) = delete;
  WebsocketClient& operator=(const WebsocketClient&) = delete;

  // 不允许移动
  WebsocketClient(WebsocketClient&&) = delete;
  WebsocketClient& operator=(WebsocketClient&&) = delete;

  /**
   * @brief 连接到WebSocket服务器
   * @param url 服务器地址 (ws:// or wss://)
   * @param auto_reconnect 是否开启自动重连，默认关闭
   * @param reconnect_delay_ms 重连延迟毫秒，默认5000ms
   * @return true 初始化成功，false 失败
   */
  bool connect(const char* url, bool auto_reconnect = false,
               uint32_t reconnect_delay_ms = 5000,const char *subprotocol="") {
    if (m_client != nullptr) {
      disconnect();
    }

    m_auto_reconnect = auto_reconnect;
    m_reconnect_delay_ms = reconnect_delay_ms;
    m_url = url;

    esp_websocket_client_config_t config = {};
    config.uri = url;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.reconnect_timeout_ms = m_reconnect_delay_ms;
    config.network_timeout_ms = 10 * 1000;
    if(subprotocol!=nullptr&&subprotocol[0]!='\0')
{
  config.subprotocol=subprotocol;
}
    m_client = esp_websocket_client_init(&config);
    if (m_client == nullptr) {
      return false;
    }

    // 设置事件回调
    esp_websocket_register_events(m_client, WEBSOCKET_EVENT_ANY,
                                  websocket_event_handler, this);

    esp_err_t err = esp_websocket_client_start(m_client);
    if (err != ESP_OK) {
      esp_websocket_client_destroy(m_client);
      m_client = nullptr;
      return false;
    }

    return true;
  }

  /**
   * @brief 断开连接并销毁客户端
   */
  void disconnect() {
    if (m_client != nullptr) {
      esp_websocket_client_stop(m_client);
      esp_websocket_client_destroy(m_client);
      m_client = nullptr;
      m_connected = false;
    }
  }

  /**
   * @brief 检查是否已连接
   * @return true 已连接
   */
  bool is_connected() const {
    if (m_client == nullptr) {
      return false;
    }
    return esp_websocket_client_is_connected(m_client) && m_connected;
  }

  /**
   * @brief 发送文本数据
   * @param text 文本字符串
   * @return true 发送成功
   */
  bool send_text(const char* text) {
    return send(text, strlen(text), WS_TRANSPORT_OPCODES_TEXT);
  }

  /**
   * @brief 发送文本数据 (std::string版本)
   * @param text 文本字符串
   * @return true 发送成功
   */
  bool send_text(const std::string& text) {
    return send(text.c_str(), text.size(), WS_TRANSPORT_OPCODES_TEXT);
  }

  /**
   * @brief 发送二进制数据
   * @param data 数据指针
   * @param len 数据长度
   * @return true 发送成功
   */
  bool send_binary(const char* data, size_t len) {
    return send(data, len, WS_TRANSPORT_OPCODES_BINARY);
  }

  /**
   * @brief 发送二进制数据 (std::string版本)
   * @param data 二进制数据
   * @return true 发送成功
   */
  bool send_binary(const std::string& data) {
    return send(data.data(), data.size(), WS_TRANSPORT_OPCODES_BINARY);
  }

  /**
   * @brief 通用发送接口
   * @param data 数据指针
   * @param len 数据长度
   * @param opcode 操作码 (WS_DATA_TEXT / WS_DATA_BINARY)
   * @return true 发送成功
   */
  bool send(const char* data, size_t len, int opcode) {
    if (!is_connected() || m_client == nullptr) {
      return false;
    }
    if (opcode == WS_TRANSPORT_OPCODES_BINARY) {
      int sent =
          esp_websocket_client_send_bin(m_client, data, len, portMAX_DELAY);
      return sent == (int)len;
    } else if (opcode == WS_TRANSPORT_OPCODES_TEXT) {
      int sent =
          esp_websocket_client_send_text(m_client, data, len, portMAX_DELAY);
      return sent == (int)len;
    }
    return false;
  }

  /**
   * @brief 发送文本帧
   * @param fin 是否是最后一帧
   * @param data 数据指针
   * @param len 数据长度
   * @return true 发送成功
   */
  bool send_text_frame(bool fin, const uint8_t* data, size_t len) {
    return send_frame(fin, WS_TRANSPORT_OPCODES_TEXT, data, len);
  }

  /**
   * @brief 发送二进制帧
   * @param fin 是否是最后一帧
   * @param data 数据指针
   * @param len 数据长度
   * @return true 发送成功
   */
  bool send_binary_frame(bool fin, const uint8_t* data, size_t len) {
    return send_frame(fin, WS_TRANSPORT_OPCODES_BINARY, data, len);
  }

  /**
   * @brief 通用帧发送接口
   * @param fin 是否是最后一帧
   * @param opcode 操作码
   * @param data 数据指针
   * @param len 数据长度
   * @return true 发送成功
   */
  bool send_frame(bool fin, ws_transport_opcodes_t opcode, const uint8_t* data,
                  size_t len) {
    if (!is_connected() || m_client == nullptr) {
      return false;
    }

    int sent = esp_websocket_client_send_with_opcode(m_client, opcode, data,
                                                     len, portMAX_DELAY);
    return sent == (int)len;
  }

  /**
   * @brief 设置连接成功回调
   * @param callback 回调函数
   */
  void on_connected(ConnectedCallback callback) {
    m_connected_callback = std::move(callback);
  }

  /**
   * @brief 设置断开连接回调
   * @param callback 回调函数
   */
  void on_disconnected(DisconnectedCallback callback) {
    m_disconnected_callback = std::move(callback);
  }

  /**
   * @brief 设置接收消息回调
   * @param callback 回调函数 (data, length)
   */
  void on_message(MessageCallback callback) {
    m_message_callback = std::move(callback);
  }

  /**
   * @brief 设置错误回调
   * @param callback 回调函数 (error_code)
   */
  void on_error(ErrorCallback callback) {
    m_error_callback = std::move(callback);
  }

  /**
   * @brief 设置是否开启自动重连
   * @param enable 是否开启
   * @param delay_ms 重连延迟
   */
  void set_auto_reconnect(bool enable, uint32_t delay_ms = 5000) {
    m_auto_reconnect = enable;
    m_reconnect_delay_ms = delay_ms;
  }

  /**
   * @brief 获取底层esp_websocket_client句柄
   * @return esp_websocket_client_handle_t
   */
  esp_websocket_client_handle_t handle() { return m_client; }

 private:
  static void websocket_event_handler(void* handler_args, esp_event_base_t base,
                                      int32_t event_id, void* event_data) {
    WebsocketClient* self = static_cast<WebsocketClient*>(handler_args);
    esp_websocket_event_data_t* data =
        static_cast<esp_websocket_event_data_t*>(event_data);

    switch (event_id) {
      case WEBSOCKET_EVENT_CONNECTED: {
        ESP_LOGI("WebsocketClient", "Websocket Connected!");
        self->m_connected = true;
        if (self->m_connected_callback) {
          self->m_connected_callback();
        }
      } break;

      case WEBSOCKET_EVENT_DISCONNECTED: {
        ESP_LOGI("WebsocketClient", "Websocket Disconnected!");

        bool was_connected = self->m_connected;
        self->m_connected = false;
        if (was_connected && self->m_disconnected_callback) {
          self->m_disconnected_callback();
        }
        // 如果开启了自动重连，尝试重连
        if (self->m_auto_reconnect && self->m_client != nullptr) {
          // 注意: esp_websocket_client会自动尝试重连，这里可以不需要额外处理
          // 如果需要自定义重连逻辑，可以在这里实现
        }

      } break;

      case WEBSOCKET_EVENT_DATA: {
        ESP_LOGI("WebsocketClient", "Websocket Data! %d", data->op_code);

        if (self->m_message_callback && data->data_len > 0) {
          self->m_message_callback((ws_transport_opcodes_t)data->op_code,
                                   data->data_ptr, data->data_len);
        }
      } break;

      case WEBSOCKET_EVENT_ERROR: {
        ESP_LOGI("WebsocketClient", "Websocket Error!");

        if (self->m_error_callback) {
          self->m_error_callback(data->error_handle);
        }
      } break;

      default:
        break;
    }
  }

  esp_websocket_client_handle_t m_client;
  std::string m_url;
  bool m_connected;
  bool m_auto_reconnect;
  uint32_t m_reconnect_delay_ms;

  ConnectedCallback m_connected_callback;
  DisconnectedCallback m_disconnected_callback;
  MessageCallback m_message_callback;
  ErrorCallback m_error_callback;
};
