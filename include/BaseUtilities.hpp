#pragma once

#include <esp_log.h>

#include <algorithm>
#include <initializer_list>
#include <string>
#include <esp_event.h>
#include "DelayExec.hpp"
#include "Timer.hpp"
#include "UUID.hpp"
#include "WiFi.hpp"
#include "FileHandle.hpp"
#include "WebsocketClient.hpp"
ESP_EVENT_DECLARE_BASE(BASE_UTILITIES_BASE);
typedef enum base_utilities_event {
  BU_WIFI_CONNECTED,
  BU_WIFI_DISCONNECTED,
} base_utilities_event;
/**
 * @brief 在方框中打印多行信息
 * @param level 日志级别
 * @param tag 日志标签
 * @param title 框顶部的标题
 * @param lines 内容行列表，使用 { "line1", "line2" } 形式传入
 */
void log_box(esp_log_level_t level, const char* tag, const char* title,
                    std::initializer_list<std::string> lines);

uint64_t millis();
class BaseUtilitiesClass {
  private:
  bool initNvsFlash();
  void markRollbackFlag();
 public:
  BaseUtilitiesClass();
  void init();
};
extern BaseUtilitiesClass BaseUtilities;