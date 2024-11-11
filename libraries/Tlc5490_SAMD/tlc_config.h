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

#ifndef TLC_CONFIG_H
#define TLC_CONFIG_H

#include <stdint.h>

/** \file
    Configuration for the Arduino Tlc5940 library.  After making changes to
    this file, delete Tlc5940.o in this folder so the changes are applied.

    A summary of all the options:
    - Number of TLCs daisy-chained: NUM_TLCS (default 1)
    - Enable/Disable VPRG functionality: VPRG_ENABLED (default 0)
    - Enable/Disable XERR functionality: XERR_ENABLED (default 0)
    - Which pins to use for bit-banging: SIN_PIN, SIN_PORT, SIN_DDR and
        SCLK_PIN, SCLK_PORT, SCLK_DDR
    - The PWM period: TLC_PWM_PERIOD (be sure to change TLC_GSCLK_PERIOD
        accordingly!)

    How to change the pin mapping:
    - Arduino digital pin 0-7  = PORTD, PD0-7
    - Arduino digital pin 8-13 = PORTB, PB0-5
    - Arduino analog pin  0-5  = PORTC, PC0-5 */

/** Bit-bang using any two i/o pins */
#define TLC_BITBANG        1

/* ------------------------ START EDITING HERE ----------------------------- */

/** Number of TLCs daisy-chained.  To daisy-chain, attach the SOUT (TLC pin 17)
    of the first TLC to the SIN (TLC pin 26) of the next.  The rest of the pins
    are attached normally.
    \note Each TLC needs it's own IREF resistor */
#define NUM_TLCS    3

/** Determines how data should be transfered to the TLCs.  Bit-banging can use
    any two i/o pins.
    - Bit-Bang = TLC_BITBANG */
#define DATA_TRANSFER_MODE    TLC_BITBANG

/* This include is down here because the files it includes needs the data
   transfer mode */
#include "pinouts/chip_includes.h"

/* Set DATA_TRANSFER_MODE to TLC_BITBANG and change the pins below if you need
   to use different pins for sin and sclk.  The defaults are defined in
   pinouts/ATmega_xx8.h for most Arduino's. */

#if DATA_TRANSFER_MODE == TLC_BITBANG
/** SIN (TLC pin 26) */
#define SIN_PIN_1        DEFAULT_BB_SIN_PIN_1
#define SIN_PIN_2        DEFAULT_BB_SIN_PIN_2
#define SIN_PORT_1       DEFAULT_BB_SIN_PORT_1
#define SIN_PORT_2       DEFAULT_BB_SIN_PORT_2
#define SIN_DDR_1        DEFAULT_BB_SIN_DDR_1
#define SIN_DDR_2        DEFAULT_BB_SIN_DDR_2
/** SCLK (TLC pin 25) */
#define SCLK_PIN_1       DEFAULT_BB_SCLK_PIN_1
#define SCLK_PIN_2       DEFAULT_BB_SCLK_PIN_2
#define SCLK_PORT_1      DEFAULT_BB_SCLK_PORT_1
#define SCLK_PORT_2      DEFAULT_BB_SCLK_PORT_2
#define SCLK_DDR_1       DEFAULT_BB_SCLK_DDR_1
#define SCLK_DDR_2       DEFAULT_BB_SCLK_DDR_2
#endif


/** If more than 16 TLCs are daisy-chained, the channel type has to be uint16_t.
    Default is uint8_t, which supports up to 16 TLCs. */
#define TLC_CHANNEL_TYPE    uint8_t


/** -------------------- Timing configuration ---------------------------------
    To achieve a target PWM frequency f, use the following equation:
    
        f = 48 MHz / (TLC_GCLK_DIV * TLC_PWM_PERIOD)

    Don't forget to also set TLC_GSCLK_PERIOD accordingly:

        TLC_GSCLK_PERIOD = (TLC_PWM_PERIOD / 4096) - 1 

    Example configurations using full duty range of 0-4095
    PWM frequency       TLC_GCLK_DIV    TLC_PWM_PERIOD      TLC_GSCLK_PERIOD
    ~976 Hz             2               24576               5
    ~5859 Hz            1               8192                1

    WARNING: this high frequency config will only use duty levels 0-2047
    PWM frequency       TLC_GCLK_DIV    TLC_PWM_PERIOD      TLC_GSCLK_PERIOD
    ~11,719 Hz          1               4096                1               
*/

/** Sets the frequency of the general clock (GCLK) by dividing the 48 MHz 
    system clock by the specified whole number factor. */
#define TLC_GCLK_DIV        2

/** Determines how long each PWM period should be, in cycles of the general
    clock. */
#define TLC_PWM_PERIOD      24576

/** Determines the period of the grayscale clock (GSCLK) timer. The effective
    period is (TLC_GSCLK_PERIOD + 1) cycles of the general clock. 
    WARNING: The minimum value TLC_GSCLK_PERIOD can take is 1. */
#define TLC_GSCLK_PERIOD    5

#endif
