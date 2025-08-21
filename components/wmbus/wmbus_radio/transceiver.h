#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/spi/spi.h"
#include "freertos/FreeRTOS.h"

#include <initializer_list>
#include <vector>

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
  bool read_in_task(uint8_t *buffer, size_t length);
  void attach_data_interrupt(void (*func)(TaskHandle_t *), TaskHandle_t *arg);

  void set_spi_delegate(spi::SPIDevice *delegate) { this->delegate_ = delegate; }
  void set_reset_pin(InternalGPIOPin *pin);
  void set_data_pin(GPIOPin *pin) { this->data_pin_ = pin; }
  void set_sync_pin(GPIOPin *pin) { this->sync_pin_ = pin; }
  void set_irq_pin(InternalGPIOPin *pin);
  void set_frequency(float frequency_mhz) { this->frequency_mhz_ = frequency_mhz; }

  void dump_config();

 protected:
  void common_setup();
  void reset();
  void spi_setup();

  uint8_t spi_transaction(uint8_t operation, uint8_t address, std::initializer_list<uint8_t> data);
  uint8_t spi_read(uint8_t address);
  void spi_write(uint8_t address, std::initializer_list<uint8_t> data);
  void spi_write(uint8_t address, uint8_t data);

  spi::SPIDevice *delegate_{nullptr};
  InternalGPIOPin *reset_pin_{nullptr};
  GPIOPin *data_pin_{nullptr};   // GDO0 for CC1101, DIO1 for SX1276
  GPIOPin *sync_pin_{nullptr};   // GDO2 for CC1101
  InternalGPIOPin *irq_pin_{nullptr};    // IRQ for SX1276
  float frequency_mhz_{868.95};
};

}  // namespace wmbus_radio
}  // namespace esphome

