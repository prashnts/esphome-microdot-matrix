#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace microdot_matrix {

class MicrodotMatrix: public PollingComponent,
                      public display::DisplayBuffer,
                      public i2c::I2CDevice {
 public:

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void data(uint8_t value);

  void initialize();
  void dump_config() override;
  void HOT display();

  void update() override;

  void fill(Color color) override;

  void setup();

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  void init_reset_();

  size_t get_buffer_length_();

  void start_command_();
  void end_command_();
  void start_data_();
  void end_data_();

  int get_width_internal() override;
  int get_height_internal() override;

  uint8_t _buf_matrix_lt[8];
  uint8_t _buf_matrix_rt[8];

  enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};

  display::DisplayType get_display_type() { return display::DisplayType::DISPLAY_TYPE_BINARY; };
};

}  // namespace microdot_matrix
}  // namespace esphome
