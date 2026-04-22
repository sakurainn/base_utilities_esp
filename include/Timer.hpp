#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
using namespace std::chrono_literals;

class Ticker;
class Timer {
  
 public:
  Timer();
  ~Timer();
  bool start_periodic(uint32_t period_ms, std::function<void()> callback);
  template <typename Rep, typename Period>
  bool start_periodic(std::chrono::duration<Rep, Period> period, std::function<void()> callback);
  bool start_once(uint32_t delay_ms, std::function<void()> callback);
  template <typename Rep, typename Period>
  bool start_once(std::chrono::duration<Rep, Period> delay, std::function<void()> callback);
  bool stop();
  operator bool() const;

 private:
  std::unique_ptr<Ticker> m_ticker;
};

template <typename Rep, typename Period>
bool Timer::start_periodic(std::chrono::duration<Rep, Period> period, std::function<void()> callback) {
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(period).count();
  return start_periodic(static_cast<uint32_t>(ms), std::move(callback));
}

template <typename Rep, typename Period>
bool Timer::start_once(std::chrono::duration<Rep, Period> delay, std::function<void()> callback) {
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delay).count();
  return start_once(static_cast<uint32_t>(ms), std::move(callback));
}
