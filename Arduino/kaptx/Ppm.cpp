#include "Ppm.h"

#include <avr/interrupt.h>
#include <Arduino.h>

// Document how this uses Timer 1 and Pin 10.

#define PPM_OUT (10)

// Values for Timer1 Config registers
#define WGM_15_1A (0x03)
#define WGM_15_1B (0x03 << 3)
#define COM1A_00 (0x00 << 6)
#define COM1B_10 (0x02 << 4)
#define COM1B_11 (0x03 << 4)
#define COM1C_00 (0x00 << 2)
#define CS1_DIV8 (0x02)
#define TIMSK1_TOIE (0x01)

#define PPM_CHANNELS (6)
#define PPM_PHASES (PPM_CHANNELS+1)
#define PPM_RESYNC_PHASE (0)
#define PPM_PULSE_WIDTH (800)  // 400uS
#define PPM_CENTER (3000)      // 1.5mS
#define PPM_RANGE (1600)       // 800uS throw, each side of center.
#define PPM_FRAME_LEN (40000)  // 20ms -> 50Hz

// ---------------------------------------------------------------------------------

struct Ppm_s {
    int phase; // which phase of PPM we are in, 0-6.  6 is long resync phase. 
    int time[7 /* PPM_PHASES */];  // width of each PPM phase (1 = 0.5uS)
    volatile bool startCycle;
} _ppm;

ISR(TIMER1_OVF_vect) 
{
    static unsigned remainder = PPM_FRAME_LEN;

    // Handle Timer 1 overflow: Start of PPM interval
    // Program total length of this phase in 0.5uS units.
    _ppm.phase += 1;
    if (_ppm.phase > PPM_CHANNELS) {
	_ppm.phase = 0;
	_ppm.startCycle = true;
    }

    // Remainder computation
    if (_ppm.phase == 0) {
	// Store remainder
	_ppm.time[_ppm.phase] = remainder;
	remainder = PPM_FRAME_LEN;
    }
    else {
	remainder -= _ppm.time[_ppm.phase];
    }

    // Program next interval time.
    OCR1A = _ppm.time[_ppm.phase];
}

// ---------------------------------------------------------------------------------------------
// Public API

Ppm::Ppm()
{
}

void Ppm::setup()
{
    pinMode(PPM_OUT, OUTPUT);

    // Init PPM phases
    int remainder = PPM_FRAME_LEN;
    for (_ppm.phase = 1; _ppm.phase <= PPM_CHANNELS; _ppm.phase++) {
	_ppm.time[_ppm.phase] = PPM_CENTER;  // 1.5mS
	remainder -= _ppm.time[_ppm.phase];
    }
    _ppm.time[PPM_RESYNC_PHASE] = remainder;
    _ppm.phase = 0;
    _ppm.startCycle = true;

    // Disable Interrupts
    cli();

    // Init for PPM Generation

    TCCR1A = WGM_15_1A | COM1A_00 | COM1B_11 | COM1C_00; // Fast PWM with OCR1A defining TOP
    TCCR1B = WGM_15_1B | CS1_DIV8;                       // Prescaler : system clock / 8.
    TCCR1C = 0;                                          // Not used
    OCR1A = _ppm.time[_ppm.phase];                           // Length of current phase
    OCR1B = PPM_PULSE_WIDTH;                             // Width of pulses
    // OCR1C = 0;                                           // Not used
    TIMSK1 = TIMSK1_TOIE;                                // interrupt on overflow (end of phase)

    // Enable interrupts
    sei();}

// Set PPM Channel to value.
// A value of zero corresponds to the center of throw, pulse width of 1500uS
// A positive value, v, corresponds to a longer pulse
void Ppm::write(int chan, int value)
{
  // silently fail if channel is out of range
  if ((chan < 1) || (chan > PPM_CHANNELS)) return;

  // silently fail if value is out of range
  if ((value < -PPM_RANGE) || (value > PPM_RANGE)) return;

  // Store this channel's time
  _ppm.time[chan] = PPM_CENTER + value;
}

// Wait until next PPM cycle starts
void Ppm::sync()
{
  // clear start cycle flag
  _ppm.startCycle = false;

  // wait for start cycle flag to be set again by ISR
  while (!_ppm.startCycle);
}
