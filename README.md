# evolver-arduino
Arduino code for the eVOLVER continuous cell culture framework.

The eVOLVER uses SAMD21 mini breakout boards that can be programmed using the Arduino IDE. These microcontroller breakout boards allow the eVOLVER to read analog signals from sensors (OD, temperature, etc.) and actuate pumps, stir, LEDs, and any other culture component that is needed.

[General Arduino resources](https://www.arduino.cc/)

[SAMD21 breakout board documentation](https://learn.sparkfun.com/tutorials/samd21-minidev-breakout-hookup-guide/samd21-mini-breakout-overview)

Guide for [programming the SAMD21](https://www.evolver.bio/t/installing-arduino-libraries-and-uploading-code/156) for the eVOLVER.

## ATtiny1634
The ATtiny1634 microcontroller is used only for the luminesence detection module, which is still under development. To program this microcontroller, use [avr-gcc](https://gcc.gnu.org/wiki/avr-gcc). 

[ATtiny1634 documentation](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8303-8-bit-AVR-Microcontroller-tinyAVR-ATtiny1634_Datasheet.pdf)
