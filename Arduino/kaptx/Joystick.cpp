#include "Joystick.h"

#include <Arduino.h>

#define JS_DEBOUNCE (5)
#define JS_MID (512)
#define JS_MAX (1023)

#define JS_NEUTRAL (350) // (450)
#define JS_PLUS (400) // (500)

#define TAN_37_5 (0.76732699)
#define TAN_33_75 (0.66817864)
#define TAN_22_5 (0.41421356)
#define TAN_11_25 (0.19891237)
#define TAN_7_5 (0.13165250)

// Public API
Joystick::Joystick(int xPin, int yPin, int butPin)
{
  // Save params
  x_pin = xPin;
  y_pin = yPin;
  but_pin = butPin;
}

void Joystick::setup()
{
  // Set button pin is input with pullup.
    pinMode(but_pin, INPUT_PULLUP);

    but = false;
    pressed = false;
    debounce = 0;
}

void Joystick::poll()
{
    bool oldbut = but;

    // Read input signals
    x = JS_MID - analogRead(x_pin);
    y = JS_MID - analogRead(y_pin);
    but = digitalRead(but_pin);

    // Update debounce timer
    if (debounce > 0) debounce--;

    if (oldbut && !but && (debounce == 0)) {
      // Button was up, is now down and debounce not in effect.
      // This is a button press
      pressed = true;

      // Start debounce timer
      debounce = JS_DEBOUNCE;
    }
}

// map joystick x, y into sectors 0-23, in standard orientation
// (0 is due "east", 6 is north, 12 west, 18 south.)
int Joystick::getIndex24()
{
    int index = 0;
    int x = this->x;
    int y = this->y;
    bool fold_y = false;
    bool fold_x = false;
    bool fold_xy = false;
    int tmp;

    // fold x, y into 0, 1 positions
    if (y < 0) {
	fold_x = true;
	y = -y;
    }
    if (x < 0) {
	fold_y = true;
	x = -x;
    }
    if (y > x) {
	fold_xy = true;
	tmp = x;
	x = y;
	y = tmp;
    }

    float t = (float)y/(float)x;
    if (t > TAN_22_5) {
	if (t > TAN_37_5) {
	    index = 3;
	}
	else {
	    index = 2;
	}
    }
    else {
	if (t > TAN_7_5) {
	    index = 1;
	}
	else {
	    index = 0;
	}
    }

    // unfold as necessary
    if (fold_xy) index = 6 - index;
    if (fold_y) index = 12 - index;
    if (fold_x) index = 24 - index;
    if (index == 24) index = 0;

    return index;
}

// map joystick x, y into sectors 0-15, in standard orientation
// (0 is due "east", 4 is north, 8 west, 12 south.)
int Joystick::getIndex16()
{
    int index = 0;
    int x = this->x;
    int y = this->y;
    bool fold_y = false;
    bool fold_x = false;
    bool fold_xy = false;
    int tmp;

    // fold x, y into 0, 1 positions
    if (y < 0) {
	fold_x = true;
	y = -y;
    }
    if (x < 0) {
	fold_y = true;
	x = -x;
    }
    if (y > x) {
	fold_xy = true;
	tmp = x;
	x = y;
	y = tmp;
    }

    float t = (float)y/(float)x;
    if (t > TAN_22_5) {
	index = 1;
    }

    // unfold as necessary
    if (fold_xy) index = 3 - index;
    if (fold_y) index = 7 - index;
    if (fold_x) index = 15 - index;

    return index;
}

// map joystick x, y into sectors 0-15, in standard orientation
// (0 is due "east", 4 is north, 8 west, 12 south.)
// difference between this and jsGetIndex16() is that these regions are centered on
// the cardinal directions.  
int Joystick::getIndex16_offs()
{
    int index = 0;
    int x = this->x;
    int y = this->y;
    bool fold_y = false;
    bool fold_x = false;
    bool fold_xy = false;
    int tmp;

    // fold x, y into 0, 1 positions
    if (y < 0) {
	fold_x = true;
	y = -y;
    }
    if (x < 0) {
	fold_y = true;
	x = -x;
    }
    if (y > x) {
	fold_xy = true;
	tmp = x;
	x = y;
	y = tmp;
    }

    float t = (float)y/(float)x;
    if (t < TAN_22_5) {    
      if (t < TAN_11_25) {
	index = 0;
      }
      else {
          index = 1;
      }
    }
    else {
      if (t < TAN_33_75) {
        index = 1;
      }
      else {
        index = 2;
      }
    }

    // unfold as necessary
    if (fold_xy) index = 4 - index;
    if (fold_y) index = 8 - index;
    if (fold_x) index = 16 - index;
    if (index == 16) index = 0;

    return index;
}

bool Joystick::wasPressed()
{
  bool retval = pressed;
  pressed = false;

  return retval;
}

bool Joystick::isOut()
{
    long r2 = ((long)x*x) + ((long)y*y);
    return r2 > (long)JS_PLUS*JS_PLUS;
}

bool Joystick::isCenter()
{
    long r2 = ((long)x*x) + ((long)y*y);
    return (r2 < (long)JS_NEUTRAL*JS_NEUTRAL);
}



