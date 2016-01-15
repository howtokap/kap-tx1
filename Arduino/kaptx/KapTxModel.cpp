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

#define ACCEL_PAN (1)
#define ACCEL_TILT (1)

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

// ----------------------------------------------------------------------------------
// Forward declaration
static void toPwm(PanTilt_t *pwm, const PanTilt_t *user);

// -----------------------------------------------------------------------------------
// KapTxModel Public methods

KapTxModel::KapTxModel()
{
    userPos.pan = 6;  // facing away from operator
    userPos.tilt = 0;  // facing horizontal.

    toPwm(&servoPos, &userPos);
    servoVel.pan = 0;
    servoVel.tilt = 0;

    shutterServo = SHUTTER_UP_POS;

    shootMode = MODE_SINGLE;
    shotsQueued = 0;

    hoVer = false;
    autokap = false;

    dispFlags = CHANGE_FLAGS;
}

void KapTxModel::setPan(int index)
{
    if (userPos.pan != index) {
	userPos.pan = index;
	dispFlags |= REFRESH_PAN_TILT;
    }
}

void KapTxModel::adjPan(int adj)
{
    userPos.pan += adj;
    if (userPos.pan > PAN_MAX) {
	userPos.pan -= (PAN_MAX+1);
    }
    if (userPos.pan < PAN_MIN) {
	userPos.pan += (PAN_MAX+1);
    }
    dispFlags |= REFRESH_PAN_TILT;
}

void KapTxModel::setTilt(int index)
{
    int oldTilt = userPos.tilt;

    // Note: TILT_MAX is at index 2, TILT_MIN at 15.
    // This is because tilt indices use standard orientation.
    if ((index <= TILT_MAX) || (index >= TILT_MIN)) {
      // adopt this tilt
      userPos.tilt = index;
    }
    else if (index < (TILT_MIN + TILT_MAX)/2) {
        // set max tilt
        userPos.tilt = TILT_MAX;
    }
    else {
        // set min tilt
        userPos.tilt = TILT_MIN;
    }

    if (userPos.tilt != oldTilt) {
	dispFlags |= REFRESH_PAN_TILT;
    }
}

// Note: this is intended to work with adjustments of +/- 1.
void KapTxModel::adjTilt(int adj)
{
    if ((adj > 0) && (userPos.tilt == TILT_MAX)) return;  // no change
    if ((adj < 0) && (userPos.tilt == TILT_MIN)) return;  // no change
    userPos.tilt += adj;
    if (userPos.tilt >= 24) userPos.tilt -= 24;
    if (userPos.tilt < 0) userPos.tilt += 24;
    dispFlags |= REFRESH_PAN_TILT;
}

void KapTxModel::getUserPos(PanTilt_t *aimPoint)
{
    *aimPoint = userPos;
}

bool KapTxModel::getHoVer() 
{
    return hoVer;
}

void KapTxModel::setHoVer(bool state)
{
    if (state != hoVer) {
	dispFlags |= REFRESH_HOVER;
    }
    hoVer = state;
}

void KapTxModel::invHoVer(bool state)
{
    if (state) {
	dispFlags |= INV_HOVER;
    }
    else {
	dispFlags &= ~INV_HOVER;
    }
    dispFlags |= REFRESH_HOVER;
}

void KapTxModel::setAuto(bool state)
{
    if (state != autokap) {
	dispFlags |= REFRESH_AUTO_COUNT;
    }
    autokap = state;
    updateLcdShutterState();
}

bool KapTxModel::getAuto()
{
    return autokap;
}

void KapTxModel::invAuto(bool state)
{
    if (state) {
	dispFlags |= INV_AUTO_COUNT;
    }
    else {
	dispFlags &= ~INV_AUTO_COUNT;
    }
    dispFlags |= REFRESH_AUTO_COUNT;
}

void KapTxModel::setDispMode(Mode_t mode)
{
    if (mode != shootMode_disp) {
	shootMode_disp = mode;
	dispFlags |= REFRESH_SHOOT_MODE;
    }
}

void KapTxModel::setModeToDispMode()
{
    shootMode = shootMode_disp;
    dispFlags |= REFRESH_SHOOT_MODE;
}

void KapTxModel::invMode(bool state)
{
    if (state) {
	dispFlags |= INV_SHOOT_MODE;
    }
    else {
	dispFlags &= ~INV_SHOOT_MODE;
    }
    dispFlags |= REFRESH_SHOOT_MODE;
}

Mode_t KapTxModel::getShootMode()
{
    return shootMode;
}

Mode_t KapTxModel::getShootModeDisp()
{
    return shootMode_disp;
}

unsigned char KapTxModel::getDispFlags()
{
    unsigned char retval = dispFlags;

    dispFlags &= ~CHANGE_FLAGS;

    return retval;
}

unsigned char KapTxModel::getLcdShutterState()
{
    return lcdShutterState;
}

bool KapTxModel::atGoalPos()
{
    PanTilt_t goal;
    getGoalPwm(&goal);

    return ((goal.pan == servoPos.pan) &&
	    (goal.tilt == servoPos.tilt) &&
	    (servoVel.pan == 0) &&
	    (servoVel.tilt == 0));
}

void KapTxModel::getGoalPwm(PanTilt_t *goal)
{
    if (shotsQueued) {
	// slew target is the head of shot queue
	*goal = shotQueue[0];
    }
    else {
	// slew target is user position
	toPwm(goal, &userPos);
    }
}

void KapTxModel::queueShot(PanTilt_t *aimPoint)
{
    PanTilt_t aimPointPwm;
    toPwm(&aimPointPwm, aimPoint);
    
    queueShotPwm(&aimPointPwm);
}

unsigned KapTxModel::getShotsQueued()
{
    return shotsQueued;
}

void KapTxModel::dequeueShot()
{
    for (int n = 0; n < SHOT_QUEUE_LEN-2; n++) {
	shotQueue[n] = shotQueue[n+1];
    }
    shotsQueued--;
    dispFlags |= REFRESH_AUTO_COUNT;
}

void KapTxModel::getServos(PanTilt_t *pos, PanTilt_t *vel)
{
    *pos = servoPos;
    *vel = servoVel;
}

void KapTxModel::setServos(PanTilt_t *pos, PanTilt_t *vel)
{
    servoPos = *pos;
    servoVel = *vel;
}

int KapTxModel::getPanPwm()
{
    return servoPos.pan;
}

int KapTxModel::getTiltPwm()
{
    return servoPos.tilt;
}

int KapTxModel::getShutterPwm()
{
    return shutterServo;
}

void KapTxModel::setShutterPwm(int pwm)
{
    shutterServo = pwm;
}

int KapTxModel::getHoVerPwm()
{
    return (hoVer ? HOVER_VERT_POS : HOVER_HOR_POS);
}

void KapTxModel::setShutterState(unsigned char state)
{
    unsigned char lcd_ss;

    shutterState = state;
    updateLcdShutterState();
}

void KapTxModel::setSlewStable(bool state)
{
    slewStable = state;
    updateLcdShutterState();
}

bool KapTxModel::getSlewStable()
{
    return slewStable;
}

// --------------------------------------------------------------------------------
// Utility methods

void KapTxModel::updateLcdShutterState()
{
  unsigned char state = 0;

  if (shutterState == SHUTTER_DOWN) {
    state = SHUTTER_STATE_TRIG;
  }
  else if ((shutterState == SHUTTER_POST) ||
           (shutterState == SHUTTER_TRIGGERED)) {
    state = SHUTTER_STATE_ACT;
  }
  else if (!autokap && slewStable) {
    state = SHUTTER_STATE_RDY;
  }
  else {
    state = SHUTTER_STATE_NO;
  }

  if (state != lcdShutterState) {
      lcdShutterState = state;
      dispFlags |= REFRESH_SHUTTER;
  }
}

void KapTxModel::queueShotPwm(PanTilt_t *aimPoint)
{
    // bail out if queue is full
    if (shotsQueued == SHOT_QUEUE_LEN) return;

    PanTilt_t *entry = shotQueue + shotsQueued;
    *entry = *aimPoint;
    shotsQueued++;
    dispFlags |= REFRESH_AUTO_COUNT;
}

static int iabs(int x)
{
    if (x < 0) {
	return -x;
    }
    return x;
}

void KapTxModel::toPwm(PanTilt_t *pwm, const PanTilt_t *user)
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
              currDelta = iabs(pwm - servoPos.pan);
          } else {
              newDelta = iabs(pwm - servoPos.pan);
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

