#include "KapTxModel.h"

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
// to PWM offsets in the range -1600 to 1600.  (Units are microseconds offset
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
#define PWM_MAX_OFFSET (1600)

// -------------------------------------------------------------
// KAP Tx Model

#define ACCEL_PAN (1)
#define ACCEL_TILT (1)

#define ANG_360 (24)
#define TILT_MIN (15)
#define TILT_MAX (2)
#define TILT_MID_DEAD (8)
#define PAN_MIN (0)
#define PAN_MAX (23)

// #define SHOOT_IDLE (0)
// #define SHOOT_ACTIVE (1)

#define NEUTRAL_DISP_TIME (25)
#define DISP_NONE (0)
#define DISP_PAN (1)
#define DISP_TILT (2)
#define DISP_MODE (3)
#define DISP_HOVER (4)

#define SLEW_STABLE (0)
#define SLEW_MOVING (1)
#define SLEW_STABILIZING (2)

#define TICKS_PER_SECOND (50)
#define SECONDS(n) (n * TICKS_PER_SECOND)
#define TIME_STABILIZING (10)
#define TIME_SHUTTER_DOWN (5)
#define TIME_SHUTTER_POST (35)
#define TIME_JS_INITIAL (25)
#define TIME_JS_REPEAT (3)

#define SHUTTER_IDLE (0)
#define SHUTTER_TRIGGERED (1)
#define SHUTTER_DOWN (2)
#define SHUTTER_POST (3)

#define SHUTTER_DOWN_POS (600)   // 1.8ms
#define SHUTTER_UP_POS (-600)    // 1.2ms
#define HOVER_HOR_POS (600)      // 1.8ms
#define HOVER_VERT_POS (-600)    // 1.2ms

#define ARRAY_LEN(a) ((sizeof(a)) / (sizeof(a[0])))

KapTxModel::KapTxModel()
{
    // TODO-DW : Init fields
}

void KapTxModel::setup()
{
    model.userPos.pan = 6;  // facing away from operator
    model.userPos.tilt = 0;  // facing horizontal.

    model.shutterServo = SHUTTER_UP_POS;

    model.shootMode = MODE_SINGLE;
    model.shotsQueued = 0;

    model.vertical = false;
    model.autokap = false;
}

unsigned char KapTxMode::getShutterLcdState()
{
  unsigned char state = 0;
  
  if (model.shutterState == SHUTTER_DOWN) {
    state = SHUTTER_STATE_TRIG;
  }
  else if ((model.shutterState == SHUTTER_POST) ||
           (model.shutterState == SHUTTER_TRIGGERED)) {
    state = SHUTTER_STATE_ACT;
  }
  else if (!model.autokap && 
           (model.slewState == SLEW_STABLE)) {
    state = SHUTTER_STATE_RDY;
  }
  else {
    state = SHUTTER_STATE_NO;
  }
  
  return state;
}

void KapTxModel::queueShot(struct PanTilt_s *aimPoint)
{
    struct PanTilt_s aimPointPwm;
    toPwm(&aimPointPwm, aimPoint);
    
    queueShotPwm(&aimPointPwm);
}

void KapTxModel::dequeueShot()
{
    for (int n = 0; n < SHOT_QUEUE_LEN-2; n++) {
	model.shotQueue[n] = model.shotQueue[n+1];
    }
    model.shotsQueued--;
}


// --------------------------------------------------------------------------------

void KapTxModel::queueShotPwm(struct PanTilt_s *aimPoint)
{
    // bail out if queue is full
    if (model.shotsQueued == SHOT_QUEUE_LEN) return;

    struct PanTilt_s *entry = model.shotQueue + model.shotsQueued;
    *entry = *aimPoint;
    model.shotsQueued++;
}

void adjPan(int adj)
{
    model.userPos.pan += adj;
    if (model.userPos.pan > PAN_MAX) {
	model.userPos.pan -= (PAN_MAX+1);
    }
    if (model.userPos.pan < PAN_MIN) {
	model.userPos.pan += (PAN_MAX+1);
    }
}

// Note: this is intended to work with adjustments of +/- 1.
void adjTilt(int adj)
{
    if ((adj > 0) && (model.userPos.tilt == TILT_MAX)) return;  // no change
    if ((adj < 0) && (model.userPos.tilt == TILT_MIN)) return;  // no change
    model.userPos.tilt += adj;
    if (model.userPos.tilt >= 24) model.userPos.tilt -= 24;
    if (model.userPos.tilt < 0) model.userPos.tilt += 24;
}

void toPwm(struct PanTilt_s *pwm, const struct PanTilt_s *user)
{
    bool foundMatch;
    int panPwm = 0;
    int tiltPwm = 0;
    unsigned currDelta = 0;
    unsigned newDelta = 0;
    
    // PAN
    foundMatch = 0;
    for (int cycle = -1; cycle <= 1; cycle++) {
      int pwm = (user->pan + cycle*ANG_360) * PWM_FACTOR_PAN + PWM_OFFSET_PAN;
      if ((pwm >= -PWM_MAX_OFFSET) && (pwm <= PWM_MAX_OFFSET)) {
          // This is a valid pwm value.
          if (!foundMatch) {
              // It's the first match found, set current delta (distance needed to move)
              panPwm = pwm;
              currDelta = abs(pwm - model.servoPos.pan);
          } else {
              newDelta = abs(pwm - model.servoPos.pan);
              if (newDelta < currDelta) {
                  // the new one is better!
                  panPwm = pwm;
                  currDelta = newDelta;
              } 
          }
      }
    }
    
    // TILT
    if (user->tilt < 12) {
      tiltPwm = user->tilt * PWM_FACTOR_TILT + PWM_OFFSET_TILT;
    }
    else {
      // treat 23, 22, ... as -1, -2, ...
      tiltPwm = (user->tilt-ANG_360) * PWM_FACTOR_TILT + PWM_OFFSET_TILT;
    }
        
    // Set servoPos components
    pwm->pan = panPwm;
    pwm->tilt = tiltPwm;
}

boolean KapTxModel::atGoalPos()
{
    struct PanTilt_s goal;
    getGoalPwm(&goal);

    return ((goal.pan == model.servoPos.pan) &&
	    (goal.tilt == model.servoPos.tilt) &&
	    (model.servoVel.pan == 0) &&
	    (model.servoVel.tilt == 0));
}

void KapTxModel::getGoalPwm(struct PanTilt_s *goal)
{
    if (model.shotsQueued) {
	// slew target is the head of shot queue
	*goal = model.shotQueue[0];
    }
    else {
	// slew target is user position
	toPwm(goal, &model.userPos);
    }
}

void KapTxModel::setDispShootMode()
{
    shootMode = shootMode_disp;
}
