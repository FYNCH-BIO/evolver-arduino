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

/** \file
    Tlc5940 class functions. */
#if defined (__SAMD21G18A__)
#include <sam.h>
#endif

#include "tlc_config.h"
#include "Tlc5940.h"

/*
  More information can be found in the full ATMEL SAMD21 Datasheet: 
  https://cdn.sparkfun.com/datasheets/Dev/Arduino/Boards/Atmel-42181-SAM-D21_Datasheet.pdf
  (All page #'s refer to this datasheet)
*/

/*
  The necessary shape and timing of the various Tlc5940 signals can be found on that Datasheet:
  http://www.ti.com/lit/ds/symlink/tlc5940.pdf 
*/

/** Pulses a pin - high then low. */
#define pulse_pin(port, pin)   port |= (1 << pin); port &=~ (1 << pin)

/** This will be true (!= 0) if update was just called and the data has not
    been latched in yet. */
volatile uint8_t tlc_needXLAT_1;
volatile uint8_t tlc_needXLAT_2;

//"booleans" to determine the position of PWM boards
volatile uint8_t pos_1 = 0;
volatile uint8_t pos_2 = 0;
volatile uint8_t isBothPWM = 0;

/** Some of the extened library will need to be called after a successful
    update. */
volatile void (*tlc_onUpdateFinished_1)(void);
volatile void (*tlc_onUpdateFinished_2)(void);
volatile void (*tlc_onUpdateFinished_3)(void);

/** Packed grayscale data, 24 bytes (16 * 12 bits) per TLC.

    Format: Lets assume we have 2 TLCs, A and B, daisy-chained with the SOUT of
    A going into the SIN of B.
    - byte 0: upper 8 bits of B.15
    - byte 1: lower 4 bits of B.15 and upper 4 bits of B.14
    - byte 2: lower 8 bits of B.0
    - ...
    - byte 24: upper 8 bits of A.15
    - byte 25: lower 4 bits of A.15 and upper 4 bits of A.14
    - ...
    - byte 47: lower 8 bits of A.0

    \note Normally packing data like this is bad practice.  But in this
    situation, shifting the data out is really fast because the format of
    the array is the same as the format of the TLC's serial interface. */
uint8_t tlc_GSData_1[NUM_TLCS * 24];
uint8_t tlc_GSData_2[NUM_TLCS * 24];

/** Don't add an extra SCLK pulse after switching from dot-correction mode. */
static uint8_t firstGSInput;

/** Interrupt called after an XLAT pulse to prevent more XLAT pulses. */
//Datasheet page 684
void TCC1_Handler()
{
  if (TCC1->INTFLAG.bit.MC0 && TCC1->INTENSET.bit.MC0)             
    {
      REG_TCC1_INTFLAG = TCC_INTFLAG_MC0;  //clear TCC1 interrupt
      if(pos_1){
        BLANK_PORT_1 |= (1 << BLANK_PIN_1); //Bring BLANK high until XLAT pulses
      }
      if(pos_2){
        BLANK_PORT_2 |= (1 << BLANK_PIN_2); //Bring BLANK high until XLAT pulses
      }
    }
  if (TCC1->INTFLAG.bit.OVF && TCC1->INTENSET.bit.OVF){
    if (isBothPWM){
      BLANK_PORT_1 &=~ (1 << BLANK_PIN_1);  // Bring BLANK low after XLAT pulses
      BLANK_PORT_2 &=~ (1 << BLANK_PIN_2);  // Bring BLANK low after XLAT pulses
      disable_XLAT_pulses_3();
      clear_XLAT_interrupt();
      tlc_needXLAT_1 = 0;
      tlc_needXLAT_2 = 0;
      if (tlc_onUpdateFinished_3) {
        set_XLAT_interrupt();
        tlc_onUpdateFinished_3();
      }
    }else if (pos_1){
      BLANK_PORT_1 &=~ (1 << BLANK_PIN_1);  // Bring BLANK low after XLAT pulses
      disable_XLAT_pulses_1();
      clear_XLAT_interrupt();
      tlc_needXLAT_1 = 0;
      if (tlc_onUpdateFinished_1) {
        set_XLAT_interrupt();
        tlc_onUpdateFinished_1();
      }
    }else if (pos_2){
      BLANK_PORT_2 &=~ (1 << BLANK_PIN_2);  // Bring BLANK low after XLAT pulses
      disable_XLAT_pulses_2();
      clear_XLAT_interrupt();
      tlc_needXLAT_2 = 0;
      if (tlc_onUpdateFinished_2) {
        set_XLAT_interrupt();
        tlc_onUpdateFinished_2();
      }
    }
  }
}

/** \defgroup ReqVPRG_ENABLED Functions that Require VPRG_ENABLED
    Functions that require VPRG_ENABLED == 1.
    You can enable VPRG by changing
    \code #define VPRG_ENABLED    0 \endcode to
    \code #define VPRG_ENABLED    1 \endcode in tlc_config.h

    You will also have to connect Arduino digital pin 6 to TLC pin 27. (The
    pin to be used can be changed in tlc_config.h).  If VPRG is not enabled,
    the TLC pin should grounded (remember to unconnect TLC pin 27 from GND
    if you do enable VPRG). */
/* @{ */ /* @} */

/** \defgroup CoreFunctions Core Libary Functions
    These function are all prefixed with "Tlc." */
/* @{ */

/** Pin i/o and Timer setup.  The grayscale register will be reset to all
    zeros, or whatever initialValue is set to and the Timers will start.
    \param initialValue = 0, optional parameter specifing the inital startup
    value */

/* PWM positions:
   1 -> PWM in left slot
   2 -> PWM in right slot
   3 -> PWM in both slots
*/
void Tlc5940::init(uint8_t pwm_val, uint16_t initialValue)
{
  // set global PWM position values
  pos_1 = pwm_val & 0x1;
  pos_2 = pwm_val & 0x2;
  if (pos_1 && pos_2){
    isBothPWM = 1;
  }  
  
  REG_TCC1_INTENSET |=  TCC_INTENSET_MC0; // set PWM interrupts

  
  //Datasheet page 95
  // Initialize GCLK 
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(3) |  // Divide the 48MHz clock source by divisor 1: 48MHz/1=48MHz
    GCLK_GENDIV_ID(4);                    // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);      // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |   // Set the duty cycle to 50/50 HIGH/LOW
    GCLK_GENCTRL_GENEN |                  // Enable GCLK4
    GCLK_GENCTRL_SRC_DFLL48M |            // Set the 48MHz clock source
    GCLK_GENCTRL_ID(4);                   // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);      // Wait for synchronization

  // Feed GCLK4 to TCC0 and TCC1
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN | // Enable GCLK4 to TCC0 and TCC1
    GCLK_CLKCTRL_GEN_GCLK4 |              // Select GCLK4
    GCLK_CLKCTRL_ID_TCC0_TCC1 ;           // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY);      // Wait for synchronization

  NVIC_SetPriority(TCC1_IRQn, 0);         // Set the Nested Vector Interrupt Controller (NVIC) priority for TC1 to 0 (highest)
  NVIC_EnableIRQ(TCC1_IRQn);              // Connect TCC1 to Nested Vector Interrupt Controller (NVIC)


  //See pinouts folder for more information
  /* Pin Setup */
  XLAT_DDR_1 |= (1 << XLAT_PIN_1);    // set XLAT as an output  (left PWM)
  BLANK_DDR_1 |= (1 << BLANK_PIN_1);  // set BLANK as an output (left PWM)
  BLANK_PORT_1 |= (1 << BLANK_PIN_1); // leave BLANK high (until the timers start) (left PWM)

  XLAT_DDR_2 |= (1 << XLAT_PIN_2);    // set XLAT as an output  (right PWM)
  BLANK_DDR_2 |= (1 << BLANK_PIN_2);  // set BLANK as an output (right PWM)
  BLANK_PORT_2 |= (1 << BLANK_PIN_2); // leave BLANK high (until the timers start) (right PWM)

  tlc_shift8_init();                  // prepare for the set command
  
  setAll(initialValue);               // set all of the channels to the inital value
  
  if (pos_1){
    disable_XLAT_pulses_1();
    tlc_needXLAT_1 = 0;
    pulse_pin(XLAT_PORT_1, XLAT_PIN_1);
  }  
  if (pos_2){
    disable_XLAT_pulses_2();
    tlc_needXLAT_2 = 0;
    pulse_pin(XLAT_PORT_2, XLAT_PIN_2);
  }  
  clear_XLAT_interrupt();

  
  /* Timer Setup */

  /* Timer 1 -  XLAT */
  //Datasheet page 726
  REG_TCC1_WAVE |= TCC_WAVE_WAVEGEN_NPWM;    // Setup normal PWM on TCC1
  while (TCC1->SYNCBUSY.bit.WAVE);           // Wait for synchronization

  //Datasheet page 729
  REG_TCC1_PER = TLC_PWM_PERIOD;             // see tlc_config.h
  while (TCC1->SYNCBUSY.bit.PER);            // Wait for synchronization
 
  if(pos_1){
    disable_XLAT_pulses_1();                 // non inverting, output on TCC1:3, XLAT //arduino pin 3 
    /* Timer 2 - GSCLK */
    enable_GSCLK_1();                        // output on TCC0:4 //arduino pin 2  
  }
  if(pos_2){
    disable_XLAT_pulses_2();                 // non inverting, output on TCC1:1, XLAT //arduino pin 9
    /* Timer 2 - GSCLK */
    enable_GSCLK_2();                        // output on TCC0:7 //arduino pin 7
  }

  REG_TCC1_CC0 = 16360;                      //set TCC1 interrupt timing
  while (TCC1->SYNCBUSY.bit.CC0);            // Wait for synchronization

  //Datasheet page 703
  REG_TCC0_WEXCTRL |= TCC_WEXCTRL_OTMX(0x2); //Output TCC0 to all pins instead of their default

  REG_TCC0_WAVE |= TCC_WAVE_WAVEGEN_NPWM;    // Setup normal PWM on TCC0

  //Datsheet page 732
  REG_TCC0_CC0 = 1;                          // duty factor (as short a pulse as possible)
  while (TCC0->SYNCBUSY.bit.CC0);            // Wait for synchronization
  REG_TCC0_PER = TLC_GSCLK_PERIOD;           // see tlc_config.h
  while (TCC0->SYNCBUSY.bit.PER);            // Wait for synchronization

  //Datasheet page 688
  REG_TCC0_CTRLA |= TCC_CTRLA_ENABLE |
    TCC_CTRLA_PRESCALER_DIV1;                // no prescale, (start pwm output)
  while (TCC0->SYNCBUSY.bit.ENABLE);         // Wait for synchronization
  REG_TCC1_CTRLA |= TCC_CTRLA_ENABLE |
    TCC_CTRLA_PRESCALER_DIV1;                // no prescale, (start pwm output)
  while (TCC1->SYNCBUSY.bit.ENABLE);         // Wait for synchronization
  
  if (pos_1){
    enable_XLAT_pulses_1();
  }
  if (pos_2){
    enable_XLAT_pulses_2();
  }
  update(); //write an update to set all of the channels to their initialized value
}

/** Clears the grayscale data array, #tlc_GSData, but does not shift in any
    data.  This call should be followed by update() if you are turning off
    all the outputs. */
void Tlc5940::clear(void)
{
  setAll(0);
}

/** Shifts in the data from the grayscale data array, #tlc_GSData.
    If data has already been shifted in this grayscale cycle, another call to
    update() will immediately return 1 without shifting in the new data.  To
    ensure that a call to update() does shift in new data, use
    \code while(Tlc.update()); \endcode
    or
    \code while(tlc_needXLAT); \endcode
    \returns 1 if there is data waiting to be latched, 0 if data was
    successfully shifted in */
uint8_t Tlc5940::update(void)
{
  if (tlc_needXLAT_1 || tlc_needXLAT_2){
    return 1;
  }

  // disable any running XLAT_pulses
  if(pos_1){
    disable_XLAT_pulses_1();
  }
  if(pos_2){
    disable_XLAT_pulses_2();
  }

  if (firstGSInput) {
    // adds an extra SCLK pulse unless we've just set dot-correction data
    firstGSInput = 0;
  }else{ // Pulse the SCLK pin and then write data on SIN to any enabled PWM board
    if(pos_1){
      pulse_pin(SCLK_PORT_1, SCLK_PIN_1);
      uint8_t *p = tlc_GSData_1;
      while (p < tlc_GSData_1 + NUM_TLCS * 24) {
  tlc_shift8(1, *p++);
  tlc_shift8(1, *p++);
  tlc_shift8(1, *p++);
      }
    }
    if(pos_2){
      pulse_pin(SCLK_PORT_2, SCLK_PIN_2);
      uint8_t *p = tlc_GSData_2;
      while (p < tlc_GSData_2 + NUM_TLCS * 24) {
  tlc_shift8(2, *p++);
  tlc_shift8(2, *p++);
  tlc_shift8(2, *p++);
      }
    }
  }

  // Re-enable and necessary XLAT pulses
  if(pos_1){
    tlc_needXLAT_1 = 1;
    enable_XLAT_pulses_1();
  }
  if(pos_2){
    tlc_needXLAT_2 = 1;
    enable_XLAT_pulses_2();  
  }
  set_XLAT_interrupt();
  return 0;
}

/** Sets channel to value in the grayscale data array, #tlc_GSData.
    \param channel (0 to #NUM_TLCS * 16 - 1).  OUT0 of the first TLC is
    channel 0, OUT0 of the next TLC is channel 16, etc.
    \param value (0-4095).  The grayscale value, 4095 is maximum.
    \see get */
void Tlc5940::set(uint8_t pwm_val, TLC_CHANNEL_TYPE channel, uint16_t value)
{
  if(pwm_val & 0x1){
    TLC_CHANNEL_TYPE index8_1 = (NUM_TLCS * 16 - 1) - channel;
    uint8_t *index12p_1 = tlc_GSData_1 + ((((uint16_t)index8_1) * 3) >> 1);
    if (index8_1 & 1) { // starts in the middle
      // first 4 bits intact | 4 top bits of value
      *index12p_1 = (*index12p_1 & 0xF0) | (value >> 8);
      // 8 lower bits of value
      *(++index12p_1) = value & 0xFF;
    } else { // starts clean
      // 8 upper bits of value
      *(index12p_1++) = value >> 4;
      // 4 lower bits of value | last 4 bits intact
      *index12p_1 = ((uint8_t)(value << 4)) | (*index12p_1 & 0xF);
    }
  }
  if(pwm_val & 0x2){
    TLC_CHANNEL_TYPE index8_2 = (NUM_TLCS * 16 - 1) - channel;
    uint8_t *index12p_2 = tlc_GSData_2 + ((((uint16_t)index8_2) * 3) >> 1);
    if (index8_2 & 1) { // starts in the middle
      // first 4 bits intact | 4 top bits of value
      *index12p_2 = (*index12p_2 & 0xF0) | (value >> 8);
      // 8 lower bits of value
      *(++index12p_2) = value & 0xFF;
    } else { // starts clean
      // 8 upper bits of value
      *(index12p_2++) = value >> 4;
      // 4 lower bits of value | last 4 bits intact
      *index12p_2 = ((uint8_t)(value << 4)) | (*index12p_2 & 0xF);
    }
  }
}

/** Gets the current grayscale value for a channel
    \param channel (0 to #NUM_TLCS * 16 - 1).  OUT0 of the first TLC is
    channel 0, OUT0 of the next TLC is channel 16, etc.
    \returns current grayscale value (0 - 4095) for channel
    \see set */
uint16_t Tlc5940::get(TLC_CHANNEL_TYPE channel)
{
  if(pos_1){
    TLC_CHANNEL_TYPE index8 = (NUM_TLCS * 16 - 1) - channel;
    uint8_t *index12p = tlc_GSData_1 + ((((uint16_t)index8) * 3) >> 1);
    return (index8 & 1)? // starts in the middle
      (((uint16_t)(*index12p & 15)) << 8) | // upper 4 bits
      *(index12p + 1)                       // lower 8 bits
      : // starts clean
      (((uint16_t)(*index12p)) << 4) |      // upper 8 bits
      ((*(index12p + 1) & 0xF0) >> 4);      // lower 4 bits
    // that's probably the ugliest ternary operator I've ever created.
  }
  if(pos_2){
    TLC_CHANNEL_TYPE index8 = (NUM_TLCS * 16 - 1) - channel;
    uint8_t *index12p = tlc_GSData_2 + ((((uint16_t)index8) * 3) >> 1);
    return (index8 & 1)? // starts in the middle
      (((uint16_t)(*index12p & 15)) << 8) | // upper 4 bits
      *(index12p + 1)                       // lower 8 bits
      : // starts clean
      (((uint16_t)(*index12p)) << 4) |      // upper 8 bits
      ((*(index12p + 1) & 0xF0) >> 4);      // lower 4 bits
    // that's probably the ugliest ternary operator I've ever created.
  }
}

/** Sets all channels to value.
    \param value grayscale value (0 - 4095) */
void Tlc5940::setAll(uint16_t value)
{
  if(pos_2){
    uint8_t firstByte = value >> 4;
    uint8_t secondByte = (value << 4) | (value >> 8);
    uint8_t *p = tlc_GSData_2;
    while (p < tlc_GSData_2 + NUM_TLCS * 24) {
      *p++ = firstByte;
      *p++ = secondByte;
      *p++ = (uint8_t)value;
    }
  }  
  if(pos_1){
    uint8_t firstByte = value >> 4;
    uint8_t secondByte = (value << 4) | (value >> 8);
    uint8_t *p = tlc_GSData_1;
    while (p < tlc_GSData_1 + NUM_TLCS * 24) {
      *p++ = firstByte;
      *p++ = secondByte;
      *p++ = (uint8_t)value;
    }
  }
}

/** Sets all the bit-bang pins to output */
void tlc_shift8_init(void)
{
  if(pos_1){
    SIN_DDR_1 |= (1 << SIN_PIN_1);     // SIN as output  (left PWM)
    SCLK_DDR_1 |= (1 << SCLK_PIN_1);   // SCLK as output (left PWM)
    SCLK_PORT_1 &=~ (1 << SCLK_PIN_1); // start SCLK low (left PWM)
  }
  if(pos_2){
    SIN_DDR_2 |= (1 << SIN_PIN_2);     // SIN as output  (right PWM)
    SCLK_DDR_2 |= (1 << SCLK_PIN_2);   // SCLK as output (right PWM)
    SCLK_PORT_2 &=~ (1 << SCLK_PIN_2); // start SCLK low (right PWM)
  }
}

/** Shifts a byte out, MSB first */
void tlc_shift8(uint8_t pwm_val, uint8_t byte)
{
  if(pwm_val & 0x1){
    for (uint8_t bit = 0x80; bit; bit >>= 1) {
      if (bit & byte) {
  SIN_PORT_1 |= (1 << SIN_PIN_1);  // set SIN high (left PWM)
      } else {
  SIN_PORT_1 &=~ (1 << SIN_PIN_1); // set SIN low (left PWM)
      }
      pulse_pin(SCLK_PORT_1, SCLK_PIN_1);
    }
  }
  if(pwm_val & 0x2){
    for (uint8_t bit = 0x80; bit; bit >>= 1) {
      if (bit & byte) {
  SIN_PORT_2 |= (1 << SIN_PIN_2);  // set SIN high (right PWM)
      } else {
  SIN_PORT_2 &=~ (1 << SIN_PIN_2); // set SIN low (right PWM)
      }
      pulse_pin(SCLK_PORT_2, SCLK_PIN_2);
    }
  }
}

/** Preinstantiated Tlc variable. */
Tlc5940 Tlc;

/*
  If you would like to share your extended functions for others to use,
  email me (acleone ~AT~ gmail.com) with the file and an example and I'll
  include them in the library.
  
  &nbsp;
  
  \section bugs Contact
  If you found a bug in the library, email me so I can fix it!
  My email is acleone ~AT~ gmail.com
  
  &nbsp;

  \section license License - GPLv3
  Copyright (c) 2009 by Alex Leone <acleone ~AT~ gmail.com>

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

