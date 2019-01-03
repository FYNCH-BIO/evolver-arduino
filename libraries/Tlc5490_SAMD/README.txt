Sparkfun Mini Breakout Tlc5940 library for eVOLVER setup

This library can be used to control 2 buses of eVOLVER PWM boards, when the following pins are connected.
(each bus can contain multiple PWM boards daisy chained together. tlc_config.h for more info)

Sparkfun      -> Tlc5940 (LEFT_PWM)
Digital pin 2 -> GSCLCK (pin 18)
Digital pin 3 -> XLAT   (pin 24)
Digital pin 4 -> SCLCK  (pin 25)
Digital pin 5 -> SIN    (pin 26)
Analog  pin 1 -> BLANK  (pin 23)


Sparkfun      -> Tlc5940 (RIGHT_PWM)
Digital pin 7 -> GSCLCK (pin 18)
Digital pin 8 -> XLAT   (pin 24)
Digital pin 9 -> SCLCK  (pin 25)
Digital pin 10 -> SIN    (pin 26)
Analog  pin 6 -> BLANK  (pin 23)

A PWM board will usually be in the left slot, view the examples folder in arduino to see how to define the slots being used.

If there is any trouble uploading code using this library to a board, try multiple times, it may resolve itself.

Currently, when a channel is set HIGH (4095) the led or etc on that channel is fully on (led at it's brightest etc).
