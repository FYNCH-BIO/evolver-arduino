/*
  THIS IS A HEAVILY MODIFIED VERSION OF THE ORIGINAL TLC5940 Library.
  IT IS INTENDED FOR USE ONLY IN THE eVOLVER SYSTEM, DEVELOPED AT 
  BOSTON UNIVERSITY BY THE ELECTRONICS DESIGN FACILITY AND KHALIL LAB.
  (Origial library copyright in next comment)
  
  This library was redesigned to run up to two separate buses of PWM boards
  in the eVOLVER system. It currently only runs on a Sparkfun DEV-13664
  (SAMD21 Mini Breakout) board.

  Modifications by Zach Collins <collins.zw ~AT~ gmail.com>
*/

/*  Copyright (c) 2009 by Alex Leone <acleone ~AT~ gmail.com>

    This file is part of the Arduino TLC5940 Library.

    The Arduino TLC5940 Library is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    The Arduino TLC5940 Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Arduino TLC5940 Library.  If not, see
    <http://www.gnu.org/licenses/>. */
#include "Arduino.h"
#ifndef AT_SAM_D21_H
#define AT_SAM_D21_H

/*
  All Arduino pin # -> SAMD21 I/O Pin # information can be found on the sparkfun Datasheet:
  https://cdn.sparkfun.com/assets/learn_tutorials/4/5/4/graphicalDatasheet-Mini.pdf
*/

/*
  More information can be found in the full ATMEL SAMD21 Datasheet: 
  https://cdn.sparkfun.com/datasheets/Dev/Arduino/Boards/Atmel-42181-SAM-D21_Datasheet.pdf
*/

//Sparkfun Datasheet page 387 for DIR and OUT information
//(They are registers that set a pin as input/output and high/low)


/** SIN_1 (Arduino digital pin 5) -> SIN (TLC pin 26) */
#define DEFAULT_BB_SIN_PIN_1      15            //I/O Pin #
#define DEFAULT_BB_SIN_PORT_1     REG_PORT_OUT0 //can set a pin high or low
#define DEFAULT_BB_SIN_DDR_1      REG_PORT_DIR0 //can set an output

/** SIN_2 (Arduino digital pin 10) -> SIN (TLC pin 26) */
#define DEFAULT_BB_SIN_PIN_2      18            //I/O Pin #
#define DEFAULT_BB_SIN_PORT_2     REG_PORT_OUT0 //can set a pin high or low
#define DEFAULT_BB_SIN_DDR_2      REG_PORT_DIR0 //can set an output

/** SCLK_1 (Arduino digital pin 4) -> SCLK (TLC pin 25) */
#define DEFAULT_BB_SCLK_PIN_1     8             //I/O Pin #
#define DEFAULT_BB_SCLK_PORT_1    REG_PORT_OUT0 //can set a pin high or low
#define DEFAULT_BB_SCLK_DDR_1     REG_PORT_DIR0 //can set an output

/** SCLK_2 (Arduino digital pin 9) -> SCLK (TLC pin 25) */
#define DEFAULT_BB_SCLK_PIN_2     7             //I/O Pin #
#define DEFAULT_BB_SCLK_PORT_2    REG_PORT_OUT0 //can set a pin high or low
#define DEFAULT_BB_SCLK_DDR_2     REG_PORT_DIR0 //can set an output

/** OC1A_1 (Arduino digital pin 3) -> XLAT (TLC pin 24) */
#define XLAT_PIN_1     9             //I/O Pin #
#define XLAT_PORT_1    REG_PORT_OUT0 //can set a pin high or low
#define XLAT_DDR_1     REG_PORT_DIR0 //can set an output

/** OC1A_2 (Arduino digital pin 8) -> XLAT (TLC pin 24) */
#define XLAT_PIN_2     6             //I/O Pin #
#define XLAT_PORT_2    REG_PORT_OUT0 //can set a pin high or low
#define XLAT_DDR_2     REG_PORT_DIR0 //can set an output

/** OC1B_1 (Arduino analog pin 1) -> BLANK (TLC pin 23) */
#define BLANK_PIN_1    8             //I/O Pin #
#define BLANK_PORT_1   REG_PORT_OUT1 //can set a pin high or low
#define BLANK_DDR_1    REG_PORT_DIR1 //can set an output

/** OC1B_2 (Arduino analog pin 0) -> BLANK (TLC pin 23) */
#define BLANK_PIN_2    2             //I/O Pin #
#define BLANK_PORT_2   REG_PORT_OUT0 //can set a pin high or low
#define BLANK_DDR_2    REG_PORT_DIR0 //can set an output

/** OC2B_1 (Arduino digital pin 2) -> GSCLK (TLC pin 18) */
#define GSCLK_PIN_1    14            //I/O Pin #
#define GSCLK_PORT_1   REG_PORT_OUT0 //can set a pin high or low
#define GSCLK_DDR_1    REG_PORT_DIR0 //can set an output

/** OC2B_2 (Arduino digital pin 7) -> GSCLK (TLC pin 18) */
#define GSCLK_PIN_2    21            //I/O Pin #
#define GSCLK_PORT_2   REG_PORT_OUT0 //can set a pin high or low
#define GSCLK_DDR_2    REG_PORT_DIR0 //can set an output

#endif

