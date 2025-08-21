#pragma once

#include <functional>
#include <string>

#include "freertos/FreeRTOS.h"

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"

#include "esphome/components/spi/spi.h"
#include "esphome/components/wmbus_common/wmbus.h"

#include "packet.h"
#include "transceiver.h"

namespace esphome {
namespace wmbus_radio {

class Radio : public Component, public spi::SPIDevice {
public:
  void set_radio(RadioTransceiver *radio) { this->radio = radio; }
  void set_radio_type(const std::string &radio_type);
  void set_reset_pin(GPIOPin *pin);
  void set_data_pin(GPIOPin *pin);
  void set_sync_pin(GPIOPin *pin);
  void set_irq_pin(GPIOPin *pin);
  void set_frequency(float frequency);

  void setup() override;
  void loop() override;
  void receive_frame();

  void add_frame_handler(std::function<void(Frame *)> &&callback);

protected:
  static void wakeup_receiver_task_from_isr(TaskHandle_t *arg);
  static void receiver_task(Radio *arg);

  RadioTransceiver *radio{nullptr};
  TaskHandle_t receiver_task_handle_{nullptr};
  QueueHandle_t packet_queue_{nullptr};

  std::vector<std::function<void(Frame *)>> handlers_;
};
} // namespace wmbus_radio
} // namespace esphome
