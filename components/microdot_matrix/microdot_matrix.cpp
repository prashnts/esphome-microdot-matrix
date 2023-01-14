#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

#include "microdot_matrix.h"
#include "microdot_font.h"

/**
 * Display Layout:
 *
 * Two LTP305 modules are driven by a IS31FL3730 matrix LED driver.
 * Each display module contain a grid of 5x7 pixels, and a decimal character.
 *
 * Pimoroni sells these as microdot pHAT (containing three drivers for a total
 * of six display modules), and as "LED dot matrix breakout" with two modules.
 *
 * This driver focuses on the latter type, as three of them are equivalent to
 * the microdot pHAT.
 *
 * The two display modules, left and right, are driven in an almost identical
 * way -- the buffer structure is different as shown below:
 *
 * left = [
 *   row: 7654321
 *     0b01111111,  # col 1, bottom = msb
 *     0b01111111,  # col 2
 *     0b01111111,  # col 3
 *     0b01111111,  # col 4
 *     0b01111111,  # col 5
 *     0b00000000,
 *     0b00000000,
 *     0b01000000   # bit 7 = decimal dot
 * ]
 * right = [
 *   col:   12345
 *     0b00011111,  # row 1
 *     0b00011111,  # row 2
 *     0b00011111,  # row 3
 *     0b00011111,  # row 4
 *     0b00011111,  # row 5
 *     0b00011111,  # row 6
 *     0b10011111,  # row 7 + bit 8 = decimal dot
 *     0b00000000
 *  ]
 *
 */

#define ISSI_ADDR_DEFAULT 0x61

#define ISSI_REG_CONFIG   0x00
#define ISSI_REG_LE       0x0D
#define ISSI_REG_BRIGHT   0x19
#define ISSI_REG_UPDATE   0x0C

#define ISSI_REG_MATRIX_R  0x01
#define ISSI_REG_MATRIX_L  0x0E

#define DEFAULT_MODE      0x18
#define DEFAULT_REG_LE    0x17  // 35mA

#define MAX_BRIGHTNESS    127
#define INIT_BRIGHTNESS   64

#define WIDTH     5
#define HEIGHT    7
#define N_MATRIX  2

namespace esphome {
namespace microdot_matrix {

static const char *const TAG = "microdot_matrix";

void MicrodotMatrix::setup() {
  ESP_LOGCONFIG(TAG, "Setting up I2C MicrodotMatrix...");

  auto err = this->write(nullptr, 0);
  if (err != i2c::ERROR_OK) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  this->write_byte(ISSI_REG_CONFIG, DEFAULT_MODE);
  this->write_byte(ISSI_REG_LE, DEFAULT_REG_LE);
  set_brightness(this->_brightness);

  for (uint8_t i=0; i < 8; i++) {
      this->_buf_matrix_lt[i] = 0;
      this->_buf_matrix_rt[i] = 0;
  }

  this->init_internal_(this->get_buffer_length_());
}

int MicrodotMatrix::get_width_internal() { return WIDTH * N_MATRIX; }
int MicrodotMatrix::get_height_internal() { return HEIGHT; }

size_t MicrodotMatrix::get_buffer_length_() {
  return size_t(this->get_width_internal()) * size_t(this->get_height_internal());
}

void HOT MicrodotMatrix::display() {
  this->write_bytes(ISSI_REG_MATRIX_L, this->_buf_matrix_lt, sizeof(this->_buf_matrix_lt));
  this->write_bytes(ISSI_REG_MATRIX_R, this->_buf_matrix_rt, sizeof(this->_buf_matrix_rt));

  this->write_byte(ISSI_REG_UPDATE, 0x01);
}

void MicrodotMatrix::dump_config() {
  LOG_DISPLAY("", "MicrodotMatrix", this);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);

  if (this->error_code_ == COMMUNICATION_FAILED) {
    ESP_LOGE(TAG, "Communication with MicrodotMatrix failed!");
  }
}

void MicrodotMatrix::update() {
  this->do_update_();
  if (this->writer_local_.has_value()) { // insert Labda function if available
    (*this->writer_local_)(*this);
  }
  this->display();
}

void HOT MicrodotMatrix::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() ||
      y >= this->get_height_internal() ||
      x < 0 ||
      y < 0) {
    return;
  }

  if (x < WIDTH) {
    // left matrix
    if (color.is_on())
      this->_buf_matrix_lt[x] |= (1 << y);
    else
      this->_buf_matrix_lt[x] &= ~(1 << y);
  } else {
    x = x - WIDTH;
    if (color.is_on())
      this->_buf_matrix_rt[y] |= (1 << x);
    else
      this->_buf_matrix_rt[y] &= ~(1 << x);
  }
}

void MicrodotMatrix::set_brightness(uint8_t value) {
  if (value > MAX_BRIGHTNESS) {
    value = MAX_BRIGHTNESS;
  }
  this->write_byte(ISSI_REG_BRIGHT, value);
}

void MicrodotMatrix::set_decimal(bool left, bool right) {
  if (left)
    this->_buf_matrix_lt[7] |= 0b01000000;
  else
    this->_buf_matrix_lt[7] &= 0b10111111;

  if (right)
    this->_buf_matrix_rt[6] |= 0b10000000;
  else
    this->_buf_matrix_rt[6] &= 0b01111111;
}

uint8_t MicrodotMatrix::printstrf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[64];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  if (ret > 0)
    this->printstr(buffer);

  return 0;
}

uint8_t MicrodotMatrix::printstr(const char *s) {
  // for each character, we look it up in font map, and print it.
  // after that, fill the remaining area with spaces.
  uint8_t pos = 0;  // current character pos.
  for (; pos < N_MATRIX && *s; pos++)
    this->printchar(pos, *s++);

  while (pos < N_MATRIX)
    this->printchar(pos, ' ');

  return 0;
}

void MicrodotMatrix::printchar(uint8_t pos, const char ch) {
  // use the lookup table to print value.
  std::map<uint16_t, DotChar>::const_iterator it = dotfont.find((uint16_t)ch);

  if (it != dotfont.end()) {
    DotChar dchar = it->second;
    for (uint8_t x = 0; x < DOT_CHAR_WIDTH; x++) {
      for (uint8_t y = 0; y < HEIGHT; y++) {
        uint8_t c = dchar.data[x] & (0b1 << y);
        Color color;
        if (c)
          color = Color::WHITE;
        else
          color = Color::BLACK;
        this->draw_absolute_pixel_internal((pos * WIDTH) + x, y, color);
      }
    }
  }
}

}  // namespace microdot_matrix
}  // namespace esphome
