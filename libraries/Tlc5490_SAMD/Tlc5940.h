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

#ifndef TLC5940_H
#define TLC5940_H

/** \file
    Tlc5940 library header file. */
#include "Arduino.h"
#include <stdint.h>
#include "tlc_config.h"

//PWM Board position defines
#define LEFT_PWM 1
#define RIGHT_PWM 2


/*
  More information can be found in the full ATMEL SAMD21 Datasheet: 
  https://cdn.sparkfun.com/datasheets/Dev/Arduino/Boards/Atmel-42181-SAM-D21_Datasheet.pdf
 */


//Datasheet page 714
/** Enables the Timer1 Overflow interrupt, which will fire after an XLAT
    pulse */
#define set_XLAT_interrupt()    REG_TCC1_INTENSET |= TCC_INTENSET_OVF

//Datasheet page 717
/** Disables any Timer1 interrupts */
#define clear_XLAT_interrupt()  REG_TCC1_INTFLAG |= TCC_INTFLAG_OVF // Clear the OVF interrupt flag 


//(LEFT_PWM DEFINES)

//Datasheet page 388
/** Enables the output of XLAT pulses */
#define enable_XLAT_pulses_1()      PORT->Group[g_APinDescription[3].ulPort].PINCFG[g_APinDescription[3].ulPin].bit.PMUXEN = 1; PORT->Group[g_APinDescription[4].ulPort].PMUX[g_APinDescription[4].ulPin >> 1].reg = PORT_PMUX_PMUXO_F; REG_TCC1_CC1 = 1; while (TCC1->SYNCBUSY.bit.CC1)
/** Disables the output of XLAT pulses */
#define disable_XLAT_pulses_1()   PORT->Group[g_APinDescription[3].ulPort].PINCFG[g_APinDescription[3].ulPin].bit.PMUXEN = 0; REG_TCC1_CC1 = 0; while (TCC1->SYNCBUSY.bit.CC1) 


//Datasheet page 388
/** Enable the GSCLK */
#define enable_GSCLK_1() PORT->Group[g_APinDescription[2].ulPort].PINCFG[g_APinDescription[2].ulPin].bit.PMUXEN = 1; PORT->Group[g_APinDescription[2].ulPort].PMUX[g_APinDescription[2].ulPin >> 1].reg = PORT_PMUX_PMUXE_F




//(RIGHT_PWM DEFINES)

//Datasheet page 388
/** Enables the output of XLAT pulses */
#define enable_XLAT_pulses_2()      PORT->Group[g_APinDescription[8].ulPort].PINCFG[g_APinDescription[8].ulPin].bit.PMUXEN = 1; PORT->Group[g_APinDescription[8].ulPort].PMUX[g_APinDescription[8].ulPin >> 1].reg = PORT_PMUX_PMUXE_E; REG_TCC1_CC1 = 1; while (TCC1->SYNCBUSY.bit.CC1) //These are a problem
/** Disables the output of XLAT pulses */
#define disable_XLAT_pulses_2()   PORT->Group[g_APinDescription[8].ulPort].PINCFG[g_APinDescription[8].ulPin].bit.PMUXEN = 0; REG_TCC1_CC1 = 0; while (TCC1->SYNCBUSY.bit.CC1) 

//Datasheet page 388
/** Enable the GSCLK */
#define enable_GSCLK_2() PORT->Group[g_APinDescription[7].ulPort].PINCFG[g_APinDescription[7].ulPin].bit.PMUXEN = 1; PORT->Group[g_APinDescription[6].ulPort].PMUX[g_APinDescription[6].ulPin >> 1].reg = PORT_PMUX_PMUXE_F | PORT_PMUX_PMUXO_F

// When both left and right boards are PWM
// Disable the output of both LEFT and RIGHT XLAT pulses
#define disable_XLAT_pulses_3()   PORT->Group[g_APinDescription[3].ulPort].PINCFG[g_APinDescription[3].ulPin].bit.PMUXEN = 0; PORT->Group[g_APinDescription[8].ulPort].PINCFG[g_APinDescription[8].ulPin].bit.PMUXEN = 0; REG_TCC1_CC1 = 0; while (TCC1->SYNCBUSY.bit.CC1) 



//Global variable definitions
extern volatile uint8_t tlc_needXLAT_1;
extern volatile uint8_t tlc_needXLAT_2;
extern volatile uint8_t pos_1;
extern volatile uint8_t pos_2;
extern volatile uint8_t isBothPWM;
extern volatile void (*tlc_onUpdateFinished_1)(void);
extern volatile void (*tlc_onUpdateFinished_2)(void);
extern volatile void (*tlc_onUpdateFinished_3)(void);
extern uint8_t tlc_GSData_1[NUM_TLCS * 24];
extern uint8_t tlc_GSData_2[NUM_TLCS * 24];

/** The main Tlc5940 class for the entire library.  An instance of this class
    will be preinstantiated as Tlc. */
class Tlc5940
{
 public:
  void init(uint8_t pwm_val, uint16_t initialValue = 0);
  void clear(void);
  uint8_t update(void);
  void set(uint8_t pwm_val,TLC_CHANNEL_TYPE channel, uint16_t value);
  uint16_t get(TLC_CHANNEL_TYPE channel);
  void setAll(uint16_t value);
};

// Functions called when updating the value of the TLC5940's channels
void tlc_shift8_init(void);
void tlc_shift8(uint8_t pwm_val, uint8_t byte);

// for the preinstantiated Tlc variable.
extern Tlc5940 Tlc;

#endif

