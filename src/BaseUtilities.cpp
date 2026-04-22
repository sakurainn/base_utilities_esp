#include "BaseUtilities.hpp"

#include <esp_timer.h>
#include <nvs_flash.h>

static const char* TAG = "BaseUtilities";
ESP_EVENT_DEFINE_BASE(BASE_UTILITIES_BASE);

esp_event_loop_handle_t event_loop;

extern void setup();
extern void loop();
extern void onEvent(void* ctx, esp_event_base_t event_base, int32_t event_id,
                    void* event_data);
BaseUtilitiesClass BaseUtilities;
bool BaseUtilitiesClass::initNvsFlash() {
  // 1. 尝试初始化 NVS
  bool flag = false;
  esp_err_t err = nvs_flash_init();

  // 2. 检查是否因为没有可用空间或版本不匹配导致失败
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // 这种情况下，我们需要擦除整个 NVS 分区
    ESP_LOGW(TAG, "NVS partition truncated or format mismatch, erasing...");

    // 3. 擦除 NVS
    ESP_ERROR_CHECK(nvs_flash_erase());

    // 4. 重新初始化
    err = nvs_flash_init();
  }

  // 5. 最终检查是否成功
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
    flag = false;
  } else {
    ESP_LOGI(TAG, "NVS initialized successfully.");
    flag = true;
  }
  return flag;
}
void BaseUtilitiesClass::markRollbackFlag() {
#ifdef CONFIG_APP_ROLLBACK_ENABLE
  if (!verifyRollbackLater()) {
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
      if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
        if (verifyOta()) {
          esp_ota_mark_app_valid_cancel_rollback();
        } else {
          ESP_LOGE(
              TAG,
              "OTA verification failed! Start rollback to the previous version "
              "...");
          esp_ota_mark_app_invalid_rollback_and_reboot();
        }
      }
    }
  }
#endif
}
BaseUtilitiesClass::BaseUtilitiesClass() {}

void BaseUtilitiesClass::init() {
  markRollbackFlag();
  initNvsFlash();

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_event_handler_instance_register(BASE_UTILITIES_BASE, ESP_EVENT_ANY_ID,
                                      onEvent, nullptr, nullptr);

}

extern "C" void app_main(void) {
  BaseUtilities.init();
  setup();
  const auto f = [](void* args) {
    while (1) {
      loop();
    }
  };
  xTaskCreate(f, "loopTask", 8192, nullptr, 1, nullptr);
}

void log_box(esp_log_level_t level, const char* tag, const char* title,
             std::initializer_list<std::string> lines) {
  // 1. 预先检查日志级别，避免不必要的计算
  if (level > esp_log_level_get(tag)) {
    return;
  }

  // 2. 计算内容的最高长度
  size_t max_content_len = 0;
  for (const auto& line : lines) {
    if (line.length() > max_content_len) {
      max_content_len = line.length();
    }
  }

  // 3. 计算边框总宽度 (考虑标题长度)
  size_t title_len = title ? strlen(title) : 0;
  size_t box_inner_width =
      (max_content_len > title_len) ? max_content_len : title_len;
  box_inner_width += 2;  // 左右各留一个空格的间距

  // 构造横向边框线字符串
  std::string line_sep = "+" + std::string(box_inner_width, '-') + "+";

  // 4. 开始打印
  // 顶部边框
  ESP_LOG_LEVEL(level, tag, "%s", line_sep.c_str());

  // 标题行 (居中)
  if (title && title_len > 0) {
    int padding = box_inner_width - title_len;
    int left_pad = padding / 2;
    int right_pad = padding - left_pad;
    // %*s 是 C 语言技巧：动态指定宽度打印空格
    ESP_LOG_LEVEL(level, tag, "|%*s%s%*s|", left_pad, "", title, right_pad, "");
    ESP_LOG_LEVEL(level, tag, "%s", line_sep.c_str());
  }

  // 内容行 (左对齐)
  for (const auto& line : lines) {
    int right_padding = box_inner_width - line.length() - 1;
    // "| " + 内容 + "填充空格" + "|"
    ESP_LOG_LEVEL(level, tag, "| %s%*s|", line.c_str(), right_padding, "");
  }

  // 底部边框
  ESP_LOG_LEVEL(level, tag, "%s", line_sep.c_str());
}

uint64_t millis() { return esp_timer_get_time() / 1000; }
