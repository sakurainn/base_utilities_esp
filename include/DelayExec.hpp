#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <chrono>
#include <cstdint>
#include <functional>

namespace detail {

struct Context {
  std::function<void()> callback;
};

inline void delay_exec_timer_callback(TimerHandle_t xTimer) {
  auto* ctx = static_cast<Context*>(pvTimerGetTimerID(xTimer));
  if (ctx->callback) {
    ctx->callback();
  }
  delete ctx;
  xTimerDelete(xTimer, portMAX_DELAY);
}

}  // namespace detail

/**
 * @brief 非阻塞延时指定毫秒后执行可调用对象（使用FreeRTOS软件定时器）
 * @param callback 要执行的可调用对象，支持lambda、std::function、函数指针
 * @param delay_ms 延时毫秒数
 * @return true 定时器启动成功，false 创建失败
 *
 * @note 这个函数立即返回，回调在FreeRTOS定时器服务任务中执行
 * @note 定时器执行一次回调后自动删除
 * @note 使用软件定时器比创建临时任务更节省资源
 */
template <typename Callable>
inline typename std::enable_if_t<std::is_invocable_v<Callable>, bool>
delay_exec(Callable&& callback, uint32_t delay_ms) {
  auto* ctx = new detail::Context{std::forward<Callable>(callback)};
  TimerHandle_t timer =
      xTimerCreate("delay_exec", pdMS_TO_TICKS(delay_ms), pdFALSE, ctx,
                   detail::delay_exec_timer_callback);
  if (timer == nullptr) {
    delete ctx;
    return false;
  }
  if (xTimerStart(timer, 0) != pdPASS) {
    xTimerDelete(timer, 0);
    delete ctx;
    return false;
  }
  return true;
}

/**
 * @brief 非阻塞延时指定毫秒后执行带用户参数的函数指针（使用FreeRTOS软件定时器）
 * @param func 函数指针
 * @param arg 用户参数，会传递给函数
 * @param delay_ms 延时毫秒数
 * @return true 定时器启动成功，false 创建失败
 *
 * @note 这个函数立即返回，回调在FreeRTOS定时器服务任务中执行
 * @note 定时器执行一次回调后自动删除
 */
template <typename T>
inline bool delay_exec(void (*func)(T*), T* arg, uint32_t delay_ms) {
  return delay_exec([func, arg]() { func(arg); }, delay_ms);
}

/**
 * @brief 非阻塞延时后执行可调用对象（std::chrono duration版本）
 * @param callback 要执行的可调用对象，支持lambda、std::function、函数指针
 * @param delay 延时（std::chrono duration，例如 2000ms）
 * @return true 定时器启动成功，false 创建失败
 */
template <typename Callable, typename Rep, typename Period>
inline typename std::enable_if_t<std::is_invocable_v<Callable>, bool>
delay_exec(Callable&& callback, std::chrono::duration<Rep, Period> delay) {
  auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(delay).count();
  return delay_exec(std::forward<Callable>(callback),
                    static_cast<uint32_t>(ms));
}

/**
 * @brief 非阻塞延时后执行带用户参数的函数指针（std::chrono duration版本）
 * @param func 函数指针
 * @param arg 用户参数，会传递给函数
 * @param delay 延时（std::chrono duration，例如 2000ms）
 * @return true 定时器启动成功，false 创建失败
 */
template <typename T, typename Rep, typename Period>
inline bool delay_exec(void (*func)(T*), T* arg,
                       std::chrono::duration<Rep, Period> delay) {
  auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(delay).count();
  return delay_exec(func, arg, static_cast<uint32_t>(ms));
}

/**
 * @brief 基于 chrono 的模板延迟函数
 * @tparam Rep 周期数值类型
 * @tparam Period 时间单位 (std::ratio)
 */
template <typename Rep, typename Period>
static inline void delay(std::chrono::duration<Rep, Period> d) {
  // 将任意时间单位转换为毫秒
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(d).count();

  // 转换为 FreeRTOS 的 Tick 数
  // pdMS_TO_TICKS 确保了在不同频率 (configTICK_RATE_HZ) 下的准确性
  vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief 重载：直接传入数字视为 Tick 数（兼容你提到的 10个tick）
 */
static inline void delay(TickType_t ticks) { vTaskDelay(ticks); }

// 定义字面量_ticks 比如1000ms_ticks 转换成Ticks
namespace ticks_literals {

/// 毫秒转换为Ticks
inline constexpr TickType_t operator""_ms_ticks(unsigned long long ms) {
  return pdMS_TO_TICKS(static_cast<uint32_t>(ms));
}

/// 秒转换为Ticks
inline constexpr TickType_t operator""_s_ticks(unsigned long long s) {
  return pdMS_TO_TICKS(static_cast<uint32_t>(s * 1000));
}

/// 毫秒转换为Ticks (简写)
inline constexpr TickType_t operator""_ms(unsigned long long ms) {
  return pdMS_TO_TICKS(static_cast<uint32_t>(ms));
}

/// 秒转换为Ticks (简写)
inline constexpr TickType_t operator""_s(unsigned long long s) {
  return pdMS_TO_TICKS(static_cast<uint32_t>(s * 1000));
}

/// 直接指定Tick数
inline constexpr TickType_t operator""_ticks(unsigned long long ticks) {
  return static_cast<TickType_t>(ticks);
}

}  // namespace ticks_literals