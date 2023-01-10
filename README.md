# esphome-microdot-matrix

A custom component for `esp-home` to drive the beautiful `LTP305` based LED Matrices from Pimoroni.
They produce this module in two forms, a standalone [LED Matrix Breakout][matrix_url] containing
two display modules, and a Raspberry Pi HAT called Microdot pHAT. 

I really love these little displays, but I wanted to free up some of my old Raspberry Pis driving
this display (thanks to the global shortage), and so I decided to make them work with ESP-Home, which
has a great API.

## Sources

This repo is built from Pimoroni's [original implementation][pimoroni_pico]. More detail to follow.

### Notes
- Based on [`max7219digit`][max7219] implementation, I discovered that we can have custom functions in lambdas.
  This works by passing the `MicrodotMatrix` object to the lambda instead of just the `DisplayBuffer`.



## Usage

(tbd)


## License

MIT.

[matrix_url]: https://shop.pimoroni.com/products/led-dot-matrix-breakout?variant=32274405621843
[max7219]: https://github.com/esphome/esphome/blob/ecac26aebaa7aa8c27f58be2fa6a1330f92bc92d/esphome/components/max7219digit/display.py
[pimoroni_pico]: https://github.com/pimoroni/pimoroni-pico/blob/main/drivers/ltp305/ltp305.cpp
