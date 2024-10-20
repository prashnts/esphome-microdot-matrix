#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace microdot_matrix {

class MicrodotMatrix;

using microdot_writer_t = std::function<void(MicrodotMatrix &)>;

class MicrodotMatrix: public display::DisplayBuffer,
                      public i2c::I2CDevice {
public:
  void set_writer(microdot_writer_t &&writer) { this->writer_local_ = writer; };

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void data(uint8_t value);

  void initialize();
  void dump_config() override;
  void HOT display();
  void update() override;
  void setup();
  void init_brightness(uint8_t value) { this->_brightness = value; };

  void set_brightness(uint8_t value);
  void set_decimal(bool left, bool right);
  void printchar(uint8_t pos, const char ch);

  uint8_t printstrf(const char *format, ...);
  uint8_t printstr(const char *s);
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

  // These hold the internal display buffer for left and right modules.
  uint8_t _buf_matrix_lt[8];
  uint8_t _buf_matrix_rt[8];
  uint8_t _brightness{64};

  enum ErrorCode { NONE = 0, COMMUNICATION_FAILED } error_code_{NONE};

  display::DisplayType get_display_type() { return display::DisplayType::DISPLAY_TYPE_BINARY; };

  optional<microdot_writer_t> writer_local_{};
};
}
}
