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

#ifndef TLC_PROGMEM_UTILS_H
#define TLC_PROGMEM_UTILS_H

/** \file
    PROGMEM utility functions for setting grayscale or dot correction data
    from PROGMEM.  See the UsingProgmem Example for an example. */

#include <avr/pgmspace.h>
#include <avr/io.h>

#include "tlc_config.h"
#include "Tlc5940.h"

void tlc_setGSfromProgmem(prog_uint8_t *gsArray);
#if VPRG_ENABLED
void tlc_setDCfromProgmem(prog_uint8_t *dcArray);
#endif

/** \addtogroup ExtendedFunctions
    \code #include "tlc_progmem_utils.h" \endcode
    - void tlc_setGSfromProgmem(prog_uint8_t *gsArray) - copies the progmem
      grayscale to current grayscale array.  Requires a
      \link Tlc5940::update Tlc.update() \endlink.
    - void tlc_setDCfromProgmem(prog_uint8_t *dcArray) - shifts the data from a
      progmem dot correction array (doesn't need an update). */
/* @{ */

/** Sets the grayscale data from an array in progmem.  This doesn't shift out
    any data: call Tlc.update().  An example:
    \code
#include "tlc_progmem_utils.h"
prog_uint8_t gsArray1[NUM_TLCS * 24] = {
  GS_DUO((4095 * 16)/16, (4095 * 15)/16), GS_DUO((4095 * 14)/16, (4095 * 13)/16),
  GS_DUO((4095 * 12)/16, (4095 * 11)/16), GS_DUO((4095 * 10)/16, (4095 * 9)/16),
  GS_DUO((4095 * 8)/16, (4095 * 7)/16), GS_DUO((4095 * 6)/16, (4095 * 5)/16),
  GS_DUO((4095 * 4)/16, (4095 * 3)/16), GS_DUO((4095 * 2)/16, (4095 * 1)/16),
};

// sometime after Tlc.init()
tlc_setGSfromProgmem(gsArray1);
Tlc.update();
    \endcode
    This would set a ramp of values from OUT0 to OUT15.  (Although the
    NUM_TLCS * 24 looks like an error, each #GS_DUO is 3 bytes).  The array
    would have to be expanded if #NUM_TLCS != 1.

    The format of the grayscale array is explained in #tlc_GSData.

    \param gsArray A progmem array of grayscale data. */
void tlc_setGSfromProgmem(prog_uint8_t *gsArray)
{
    prog_uint8_t *gsArrayp = gsArray;
    uint8_t *gsDatap = tlc_GSData;
    while (gsDatap < tlc_GSData + NUM_TLCS * 24) {
        *gsDatap++ = pgm_read_byte(gsArrayp++);
        *gsDatap++ = pgm_read_byte(gsArrayp++);
        *gsDatap++ = pgm_read_byte(gsArrayp++);
    }
}

/* @} */

#endif

