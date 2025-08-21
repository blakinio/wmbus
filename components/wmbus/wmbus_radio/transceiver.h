#pragma once
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace wmbus_radio {

class Frame {
public:
  Frame(const std::vector<uint8_t> &data, int8_t rssi) : data_(data), rssi_(rssi) {}
  
  const std::vector<uint8_t> &data() const { return data_; }
  int8_t rssi() const { return rssi_; }
  
  std::string as_hex() const {
    std::string result;
    for (uint8_t byte : data_) {
      result += str_sprintf("%02X", byte);
    }
    return result;
  }
  
  std::string as_rtlwmbus() const {
    // Format for rtl_wmbus compatibility
    return str_sprintf("T1;%d;%s", rssi_, as_hex().c_str());
  }

private:
  std::vector<uint8_t> data_;
  int8_t rssi_;
};

class RadioTransceiver {
public:
  virtual ~RadioTransceiver() = default;
  
  // Pure virtual methods that must be implemented
  virtual void setup() = 0;
  virtual optional<uint8_t> read() = 0;
  virtual void restart_rx() = 0;
  virtual int8_t get_rssi() = 0;
  virtual const char *get_name() = 0;
  
  // Common methods
  void set_spi_delegate(spi::SPIDevice *delegate) { this->delegate_ = delegate; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }
  void set_data_pin(GPIOPin *pin) { this->data_pin_ = pin; }
  void set_sync_pin(GPIOPin *pin) { this->sync_pin_ = pin; }
  void set_irq_pin(GPIOPin *pin) { this->irq_pin_ = pin; }
  void set_frequency(float frequency_mhz) { this->frequency_mhz_ = frequency_mhz; }
  
  // Helper method for SPI transactions
  uint8_t spi_transaction(uint8_t cmd, uint8_t address, const std::vector<uint8_t> &data) {
    this->delegate_->begin_transaction();
    this->delegate_->transfer(cmd | address);
    uint8_t result = 0;
    for (uint8_t byte : data) {
      result = this->delegate_->transfer(byte);
    }
    this->delegate_->end_transaction();
    return result;
  }

protected:
  // Common setup for all transceivers
  void common_setup() {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->setup();
    }
    if (this->data_pin_ != nullptr) {
      this->data_pin_->setup();
    }
    if (this->sync_pin_ != nullptr) {
      this->sync_pin_->setup();
    }
    if (this->irq_pin_ != nullptr) {
      this->irq_pin_->setup();
    }
  }
  
  // Reset the radio chip
  void reset() {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->digital_write(false);
      delayMicroseconds(100);
      this->reset_pin_->digital_write(true);
      delayMicroseconds(100);
    }
  }
  
  spi::SPIDevice *delegate_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *data_pin_{nullptr};  // GDO0 for CC1101, DIO1 for SX1276
  GPIOPin *sync_pin_{nullptr};  // GDO2 for CC1101
  GPIOPin *irq_pin_{nullptr};   // IRQ for SX1276
  float frequency_mhz_{868.95};
};

} // namespace wmbus_radio
} // namespace esphome