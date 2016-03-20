#ifndef SERVOS_H
#define SERVOS_H

// This file contains parameters you may need to change to adapt to your own rig and controller.

// Pan and Tilt Servo tuning
// The controller software represents angles with integers from 0 to 23.
// 0 represents 0 degrees (pan to the right, tilt is horizontal)
// 6 represents 90 degrees, pan downwind, tilt would be up (not allowed)
// 12 represents 180 degrees, pan left, tilt horizontal but backwards (invalid)
// 18 represents 270 degrees, pan upwind, tilt straight down.
// [Note: valid tilt angles are +30 degrees to -135
//        in this scheme, these correspond to 2, 1, 0, 23, 22, ..., 15.]
//
// To move the servos to the corresponding positions, these need to be converted
// to PWM offsets in the range -1600 to 1600.  (Units are half-microsecond offset
// from center position)
// The formulas used are: 
//     PAN PWM = <pan> * PWM_FACTOR_PAN + PWM_OFFSET_PAN.
//     TILT PWM = <tilt> * PWM_FACTOR_TILT + PWM_OFFSET_TILT.
//
// The values provided below are correct for my rig but probably not yours.
// You will need to change them.
//
// To find the correct values, try this procedure:
// Step 1 : Determine offsets.
//      Set PWM_FACTOR_PAN and PWM_FACTOR_TILT to 0.
//      Run the software and see where the servos aim just after power on.
//      Then, modify PWM_OFFSET_PAN and PWM_OFFSET_TILT and retry this.
//      Keep adjusting these until the tilt is horizontal and the pan is to
//           the right.
//      (The offsets should stay in the range -1600 to 1600)
//
// Step 2 : Determine factors.
//      Now set the PWM_FACTOR_PAN and PWM_FACTOR_TILT parameters to 100.
//      Run the software, then adjust aim point on the display to be 
//           pan straight away from you, tilt straight down.
//      Is the rig pointing there?  No, adjust the factors and try again.
//      If the rig moves the wrong way, the factor will need to change sign.
//      
// Once the factors are correct, the pan and tilt of the rig should 
// match the controller's display for all angles.
//      

#define PWM_FACTOR_PAN (133)
#define PWM_OFFSET_PAN (-800)
#define PWM_FACTOR_TILT (265)
#define PWM_OFFSET_TILT (893)

// Shutter up/down positions
#define SHUTTER_DOWN_PWM (600)  // +300uS from center
#define SHUTTER_UP_PWM (-600)   // -300uS from center

// HoVer (Portrait/Landscape) servo positions
#define HOVER_HOR_PWM (600)     // +300uS from center
#define HOVER_VERT_PWM (-600)   // -300uS from center

// ----------------------------------------------------------------------------------------
// Joystick options

// Pins for Joystick.
#define JS_BUTTON (9)
#define JS_X (A0)
#define JS_Y (A1)

// If your Joystick is wired with X or Y direction reversed, change these from 1 to -1
#define JS_DIR_X (1)  // (-1) if X direction should be reversed
#define JS_DIR_Y (1)  // (-1) if Y direction should be reversed

// ----------------------------------------------------------------------------------------
// Timing parameters for movement and shootint cycles

// All of the following timing values are in units of 50Hz ticks.
// That is, a value of 1 represents 20ms.  50 represents 1 second.

// Time between servo stops moving taking a photo.  [50Hz ticks]
#define TIME_STABILIZING (10)  // 0.2 S

// Time shutter servo stays pressed.
#define TIME_SHUTTER_DOWN (5)  // 100ms, fast enough to trigger GentLED

// Time after releasing shutter before servos are allowed to move again.
// This is time required for the camera to focus and shoot.
#define TIME_SHUTTER_POST (35) // 700ms, Needs to be longer if autofocus used on EOS M.

#endif
