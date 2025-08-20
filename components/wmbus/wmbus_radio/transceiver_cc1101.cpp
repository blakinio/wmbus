#include "transceiver_cc1101.h"

#include "esphome/core/log.h"

namespace esphome {
namespace wmbus_radio {

static const char *TAG = "CC1101";
static constexpr uint32_t F_OSC = 26000000; // 26 MHz crystal

void CC1101::strobe_(uint8_t command) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(command);
  this->delegate_->end_transaction();
}

uint8_t CC1101::read_register_(uint8_t address) {
  return this->spi_transaction(0x80, address, {0});
}

void CC1101::write_register_(uint8_t address, uint8_t value) {
  this->spi_transaction(0x00, address, {value});
}

void CC1101::setup() {
  this->common_setup();

  ESP_LOGV(TAG, "Setup");
  this->reset();

  ESP_LOGVV(TAG, "configuring GDO pins");
  // GDO2 unused, GDO0 for RX FIFO threshold/packets
  write_register_(0x00, 0x06); // IOCFG2
  write_register_(0x02, 0x00); // IOCFG0

  ESP_LOGVV(TAG, "setting sync word");
  write_register_(0x04, 0x54);
  write_register_(0x05, 0x3D);

  ESP_LOGVV(TAG, "disable CRC and set infinite packet length");
  write_register_(0x07, 0x00); // PKTCTRL1
  write_register_(0x08, 0x02); // PKTCTRL0
  write_register_(0x06, 0x00); // PKTLEN

  ESP_LOGVV(TAG, "setting frequency");
  const uint32_t frequency = 868950000; // ~868 MHz
  uint32_t frf = ((uint64_t)frequency * (1 << 16)) / F_OSC;
  write_register_(0x0D, BYTE(frf, 2));
  write_register_(0x0E, BYTE(frf, 1));
  write_register_(0x0F, BYTE(frf, 0));

  ESP_LOGVV(TAG, "setting bitrate and deviation");
  write_register_(0x10, 0x8B); // MDMCFG4: 200kHz BW, DRATE_E=11
  write_register_(0x11, 0xF8); // MDMCFG3: DRATE_M
  write_register_(0x12, 0x13); // MDMCFG2: 2-FSK, sync word
  write_register_(0x13, 0x22); // MDMCFG1: 4-byte preamble
  write_register_(0x14, 0x00); // MDMCFG0
  write_register_(0x15, 0x50); // DEVIATN: ~50kHz

  ESP_LOGVV(TAG, "start RX");
  strobe_(0x34); // SRX

  ESP_LOGV(TAG, "CC1101 setup done");
}

optional<uint8_t> CC1101::read() {
  uint8_t rxbytes = this->spi_transaction(0xC0, 0x3B, {0}); // RXBYTES status
  if ((rxbytes & 0x7F) == 0)
    return {};

  this->delegate_->begin_transaction();
  this->delegate_->transfer(0x3F | 0xC0); // burst read from RX FIFO
  uint8_t byte = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return byte;
}

void CC1101::restart_rx() {
  strobe_(0x36); // SIDLE
  strobe_(0x3A); // SFRX
  strobe_(0x34); // SRX
}

int8_t CC1101::get_rssi() {
  uint8_t raw = this->spi_transaction(0xC0, 0x34, {0}); // RSSI status register
  int16_t signed_raw = raw >= 128 ? raw - 256 : raw;
  return (signed_raw / 2) - 74;
}

const char *CC1101::get_name() { return TAG; }

} // namespace wmbus_radio
} // namespace esphome
