#include "Timer.hpp"

#include "Ticker.h"
Timer::Timer() { m_ticker = std::make_unique<Ticker>(); }
Timer::~Timer() { m_ticker->detach(); }

bool Timer::start_periodic(uint32_t period_ms, std::function<void()> callback) {
  if (!m_ticker) return false;
  m_ticker->attach_ms(period_ms, std::move(callback));
  return true;
}

bool Timer::start_once(uint32_t delay_ms, std::function<void()> callback) {
  if (!m_ticker) return false;
  m_ticker->once_ms(delay_ms, std::move(callback));
  return true;
}

bool Timer::stop() {
  if (!m_ticker) return false;
  m_ticker->detach();
  return true;
}

Timer::operator bool() const { return m_ticker && m_ticker->active(); }
