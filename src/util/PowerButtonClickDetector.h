#pragma once

#include <Arduino.h>
#include <cstdint>

class PowerButtonClickDetector {
 public:
  void update(bool wasReleased) {
    finalizedClicks_ = 0;
    if (wasReleased) {
      pendingClicks_++;
      lastClickMs_ = millis();
    }
    if (pendingClicks_ > 0 && millis() - lastClickMs_ > MULTI_CLICK_TIMEOUT_MS) {
      finalizedClicks_ = pendingClicks_ > MAX_CLICKS ? MAX_CLICKS : pendingClicks_;
      pendingClicks_ = 0;
    }
  }

  uint8_t getClickCount() const { return finalizedClicks_; }
  void clearFinalized() const { finalizedClicks_ = 0; }
  void reset() {
    pendingClicks_ = 0;
    finalizedClicks_ = 0;
    lastClickMs_ = 0;
  }

 private:
  mutable uint8_t finalizedClicks_ = 0;
  uint8_t pendingClicks_ = 0;
  unsigned long lastClickMs_ = 0;
  static constexpr unsigned long MULTI_CLICK_TIMEOUT_MS = 400;
  static constexpr uint8_t MAX_CLICKS = 3;
};
