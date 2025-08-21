#pragma once
#include "transceiver.h"

namespace esphome {
namespace wmbus_radio {

class CC1101 : public RadioTransceiver {
public:
  void setup() override;
  optional<uint8_t> read() override;
  void restart_rx() override;
  int8_t get_rssi() override;
  const char *get_name() override;

private:
  void strobe_(uint8_t command);
  uint8_t read_register_(uint8_t address);
  void write_register_(uint8_t address, uint8_t value);
  void setup_rf_settings_();
  void flush_rx_fifo_();
  void flush_tx_fifo_();
  uint8_t get_chip_version_();
  bool is_fifo_available_();
};

} // namespace wmbus_radio
} // namespace esphome