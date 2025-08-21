#include "transceiver_cc1101.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace wmbus_radio {

static const char *TAG = "CC1101";

// CC1101 Register definitions
#define CC1101_IOCFG2       0x00  // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01  // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02  // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03  // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04  // Sync word, high byte
#define CC1101_SYNC0        0x05  // Sync word, low byte
#define CC1101_PKTLEN       0x06  // Packet length
#define CC1101_PKTCTRL1     0x07  // Packet automation control
#define CC1101_PKTCTRL0     0x08  // Packet automation control
#define CC1101_ADDR         0x09  // Device address
#define CC1101_CHANNR       0x0A  // Channel number
#define CC1101_FSCTRL1      0x0B  // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C  // Frequency synthesizer control
#define CC1101_FREQ2        0x0D  // Frequency control word, high byte
#define CC1101_FREQ1        0x0E  // Frequency control word, middle byte
#define CC1101_FREQ0        0x0F  // Frequency control word, low byte
#define CC1101_MDMCFG4      0x10  // Modem configuration
#define CC1101_MDMCFG3      0x11  // Modem configuration
#define CC1101_MDMCFG2      0x12  // Modem configuration
#define CC1101_MDMCFG1      0x13  // Modem configuration
#define CC1101_MDMCFG0      0x14  // Modem configuration
#define CC1101_DEVIATN      0x15  // Modem deviation setting
#define CC1101_MCSM2        0x16  // Main Radio Control State Machine config
#define CC1101_MCSM1        0x17  // Main Radio Control State Machine config
#define CC1101_MCSM0        0x18  // Main Radio Control State Machine config
#define CC1101_FOCCFG       0x19  // Frequency Offset Compensation config
#define CC1101_BSCFG        0x1A  // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B  // AGC control
#define CC1101_AGCCTRL1     0x1C  // AGC control
#define CC1101_AGCCTRL0     0x1D  // AGC control
#define CC1101_WOREVT1      0x1E  // High byte Event 0 timeout
#define CC1101_WOREVT0      0x1F  // Low byte Event 0 timeout
#define CC1101_WORCTRL      0x20  // Wake On Radio control
#define CC1101_FREND1       0x21  // Front end RX configuration
#define CC1101_FREND0       0x22  // Front end TX configuration
#define CC1101_FSCAL3       0x23  // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24  // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25  // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26  // Frequency synthesizer calibration

// Command strobes
#define CC1101_SRES         0x30  // Reset chip
#define CC1101_SFSTXON      0x31  // Enable/calibrate freq synthesizer
#define CC1101_SXOFF        0x32  // Turn off crystal oscillator
#define CC1101_SCAL         0x33  // Calibrate freq synthesizer & disable
#define CC1101_SRX          0x34  // Enable RX
#define CC1101_STX          0x35  // Enable TX
#define CC1101_SIDLE        0x36  // Exit RX / TX
#define CC1101_SAFC         0x37  // AFC adjustment of freq synthesizer
#define CC1101_SWOR         0x38  // Start automatic RX polling sequence
#define CC1101_SPWD         0x39  // Enter pwr down mode when CSn goes hi
#define CC1101_SFRX         0x3A  // Flush the RX FIFO buffer
#define CC1101_SFTX         0x3B  // Flush the TX FIFO buffer
#define CC1101_SWORRST      0x3C  // Reset real time clock
#define CC1101_SNOP         0x3D  // No operation

// Status registers
#define CC1101_PARTNUM      0x30  // Part number
#define CC1101_VERSION      0x31  // Current version number
#define CC1101_FREQEST      0x32  // Frequency offset estimate
#define CC1101_LQI          0x33  // Demodulator estimate for link quality
#define CC1101_RSSI         0x34  // Received signal strength indication
#define CC1101_MARCSTATE    0x35  // Control state machine state
#define CC1101_WORTIME1     0x36  // High byte of WOR timer
#define CC1101_WORTIME0     0x37  // Low byte of WOR timer
#define CC1101_PKTSTATUS    0x38  // Current GDOx status and packet status
#define CC1101_VCO_VC_DAC   0x39  // Current setting from PLL cal module
#define CC1101_TXBYTES      0x3A  // Underflow and # of bytes in TXFIFO
#define CC1101_RXBYTES      0x3B  // Overflow and # of bytes in RXFIFO

// FIFO access
#define CC1101_TXFIFO       0x3F  // TX FIFO
#define CC1101_RXFIFO       0x3F  // RX FIFO

// Crystal frequency
static constexpr uint32_t F_OSC = 26000000; // 26 MHz crystal

#define BYTE(val, n) (((val) >> (8 * (n))) & 0xFF)

void CC1101::strobe_(uint8_t command) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(command);
  this->delegate_->end_transaction();
}

uint8_t CC1101::read_register_(uint8_t address) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(0x80 | address); // Read bit + address
  uint8_t value = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  return value;
}

void CC1101::write_register_(uint8_t address, uint8_t value) {
  this->delegate_->begin_transaction();
  this->delegate_->transfer(address); // Write bit (0) + address
  this->delegate_->transfer(value);
  this->delegate_->end_transaction();
}

void CC1101::setup_rf_settings_() {
  // WMBus specific configuration
  // Based on TI Application Note for wMBus with CC1101
  
  // GDO2 output pin configuration - not used
  write_register_(CC1101_IOCFG2, 0x2E);  // High impedance (3-state)
  
  // GDO0 output pin configuration - RX FIFO threshold reached
  write_register_(CC1101_IOCFG0, 0x00);  // Assert when RX FIFO threshold reached
  
  // RX FIFO and TX FIFO thresholds - 4 bytes in FIFO
  write_register_(CC1101_FIFOTHR, 0x00);  // 4 bytes in TX FIFO, 60 bytes in RX FIFO
  
  // Sync word for wMBus T-mode
  write_register_(CC1101_SYNC1, 0x54);
  write_register_(CC1101_SYNC0, 0x3D);
  
  // Infinite packet length mode
  write_register_(CC1101_PKTLEN, 0x00);
  
  // Packet automation control
  write_register_(CC1101_PKTCTRL1, 0x00); // No address check, no append status
  write_register_(CC1101_PKTCTRL0, 0x02); // Infinite packet length mode, CRC disabled
  
  // Device address - not used in infinite packet mode
  write_register_(CC1101_ADDR, 0x00);
  
  // Channel number
  write_register_(CC1101_CHANNR, 0x00);
  
  // Frequency synthesizer control
  write_register_(CC1101_FSCTRL1, 0x06); // IF frequency
  write_register_(CC1101_FSCTRL0, 0x00); // Frequency offset
  
  // Modem configuration for wMBus 32.768 kBaud
  write_register_(CC1101_MDMCFG4, 0x8B); // Channel bandwidth ~203 kHz, DRATE_E=11
  write_register_(CC1101_MDMCFG3, 0xF8); // DRATE_M=248, data rate ~32.768 kBaud
  write_register_(CC1101_MDMCFG2, 0x13); // 2-FSK, sync word 16/16 bits detected
  write_register_(CC1101_MDMCFG1, 0x22); // 4 preamble bytes, channel spacing
  write_register_(CC1101_MDMCFG0, 0xF8); // Channel spacing
  
  // Modem deviation for wMBus ±50 kHz
  write_register_(CC1101_DEVIATN, 0x50);
  
  // Main Radio Control State Machine configuration
  write_register_(CC1101_MCSM2, 0x07);   // RX timeout behavior
  write_register_(CC1101_MCSM1, 0x30);   // CCA mode, RX->IDLE, TX->IDLE
  write_register_(CC1101_MCSM0, 0x18);   // Auto calibrate from IDLE to RX/TX
  
  // Frequency Offset Compensation Configuration
  write_register_(CC1101_FOCCFG, 0x16);  // FOC settings
  
  // Bit synchronization Configuration
  write_register_(CC1101_BSCFG, 0x6C);   // Bit sync settings
  
  // AGC control
  write_register_(CC1101_AGCCTRL2, 0x43); // AGC settings for optimal sensitivity
  write_register_(CC1101_AGCCTRL1, 0x40);
  write_register_(CC1101_AGCCTRL0, 0x91);
  
  // Front end RX/TX configuration
  write_register_(CC1101_FREND1, 0x56);   // Front end RX configuration
  write_register_(CC1101_FREND0, 0x10);   // Front end TX configuration
  
  // Frequency synthesizer calibration (keep defaults)
  write_register_(CC1101_FSCAL3, 0xE9);
  write_register_(CC1101_FSCAL2, 0x2A);
  write_register_(CC1101_FSCAL1, 0x00);
  write_register_(CC1101_FSCAL0, 0x1F);
}

void CC1101::flush_rx_fifo_() {
  strobe_(CC1101_SFRX);
}

void CC1101::flush_tx_fifo_() {
  strobe_(CC1101_SFTX);
}

uint8_t CC1101::get_chip_version_() {
  return read_register_(CC1101_VERSION | 0xC0); // Burst read for status registers
}

bool CC1101::is_fifo_available_() {
  uint8_t rxbytes = read_register_(CC1101_RXBYTES | 0xC0); // Status register
  return (rxbytes & 0x7F) > 0; // Check if FIFO has data (ignore overflow bit)
}

void CC1101::setup() {
  this->common_setup();
  
  ESP_LOGD(TAG, "Setting up CC1101...");
  
  // Reset the chip
  this->reset();
  delayMicroseconds(1000); // Wait for reset to complete
  
  // Check if chip is responding
  uint8_t version = get_chip_version_();
  if (version == 0x00 || version == 0xFF) {
    ESP_LOGE(TAG, "CC1101 not responding! Check connections. Version: 0x%02X", version);
    return;
  }
  
  ESP_LOGD(TAG, "CC1101 detected, version: 0x%02X", version);
  
  // Configure all RF settings for wMBus
  setup_rf_settings_();
  
  // Set radio frequency
  const uint32_t frequency_hz =
      static_cast<uint32_t>(this->frequency_mhz_ * 1e6f);
  uint32_t frf = ((uint64_t) frequency_hz * (1 << 16)) / F_OSC;
  write_register_(CC1101_FREQ2, BYTE(frf, 2));
  write_register_(CC1101_FREQ1, BYTE(frf, 1));
  write_register_(CC1101_FREQ0, BYTE(frf, 0));

  // Read back frequency registers to log the actual frequency set
  uint8_t freq2 = read_register_(CC1101_FREQ2);
  uint8_t freq1 = read_register_(CC1101_FREQ1);
  uint8_t freq0 = read_register_(CC1101_FREQ0);
  uint32_t frf_actual =
      (static_cast<uint32_t>(freq2) << 16) |
      (static_cast<uint32_t>(freq1) << 8) |
      freq0;
  uint32_t frequency_actual =
      ((uint64_t) frf_actual * F_OSC) >> 16;
  ESP_LOGD(TAG, "Frequency set to %u Hz [0x%02X%02X%02X]",
           frequency_actual, freq2, freq1, freq0);
  
  // Calibrate frequency synthesizer
  strobe_(CC1101_SCAL);
  delayMicroseconds(750); // Wait for calibration
  
  // Flush FIFOs
  flush_rx_fifo_();
  flush_tx_fifo_();
  
  // Start receiving
  strobe_(CC1101_SRX);
  delayMicroseconds(100);
  
  // Verify we're in RX mode
  uint8_t marcstate = read_register_(CC1101_MARCSTATE | 0xC0);
  ESP_LOGD(TAG, "MARC state after SRX: 0x%02X", marcstate);
  
  ESP_LOGI(TAG, "CC1101 setup completed successfully");
}

optional<uint8_t> CC1101::read() {
  if (!is_fifo_available_()) {
    return {};
  }
  
  // Read one byte from RX FIFO
  this->delegate_->begin_transaction();
  this->delegate_->transfer(CC1101_RXFIFO | 0xC0); // Burst read from RX FIFO
  uint8_t byte = this->delegate_->transfer(0x00);
  this->delegate_->end_transaction();
  
  return byte;
}

void CC1101::restart_rx() {
  // Go to IDLE state
  strobe_(CC1101_SIDLE);
  delayMicroseconds(100);
  
  // Flush RX FIFO
  flush_rx_fifo_();
  
  // Restart RX
  strobe_(CC1101_SRX);
  delayMicroseconds(100);
  
  ESP_LOGVV(TAG, "RX restarted");
}

int8_t CC1101::get_rssi() {
  // Read RSSI from status register
  uint8_t rssi_raw = read_register_(CC1101_RSSI | 0xC0);
  
  // Convert to dBm according to CC1101 datasheet
  int16_t rssi_dbm;
  if (rssi_raw >= 128) {
    rssi_dbm = (rssi_raw - 256) / 2 - 74;
  } else {
    rssi_dbm = rssi_raw / 2 - 74;
  }
  
  return (int8_t) rssi_dbm;
}

const char *CC1101::get_name() { 
  return TAG; 
}

} // namespace wmbus_radio
} // namespace esphome