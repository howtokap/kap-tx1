#include "Model.h"
#include "Tuning.h"
#include "Controller.h"

#define PWM_MAX_OFFSET (1600)

// ----------------------------------------------------------------------------------
// Forward declaration
static void toPwm(PanTilt_t *pwm, const PanTilt_t *user);

// -----------------------------------------------------------------------------------
// Model Public methods

Model::Model()
{
    userPos.pan = 6;  // facing away from operator
    userPos.tilt = 0;  // facing horizontal.

    toPwm(&servoPos, &userPos);
    servoVel.pan = 0;
    servoVel.tilt = 0;
    shutterPressed = false;

    shootMode = MODE_SINGLE;
    shotsQueued = 0;

    autokap = false;

    dispFlags = CHANGE_FLAGS;
}

void Model::setPan(int index)
{
    if (userPos.pan != index) {
	userPos.pan = index;
	dispFlags |= REFRESH_PAN_TILT;
    }
}

void Model::adjPan(int adj)
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

void Model::setTilt(int index)
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
void Model::adjTilt(int adj)
{
    if ((adj > 0) && (userPos.tilt == TILT_MAX)) return;  // no change
    if ((adj < 0) && (userPos.tilt == TILT_MIN)) return;  // no change
    userPos.tilt += adj;
    if (userPos.tilt >= 24) userPos.tilt -= 24;
    if (userPos.tilt < 0) userPos.tilt += 24;
    dispFlags |= REFRESH_PAN_TILT;
}

void Model::getUserPos(PanTilt_t *aimPoint)
{
    *aimPoint = userPos;
}

bool Model::getHoVer() 
{
    return hoVer;
}

void Model::setHoVer(bool state)
{
    if (state != hoVer) {
	dispFlags |= REFRESH_HOVER;
    }
    hoVer = state;
}

void Model::invHoVer(bool state)
{
    if (state) {
	dispFlags |= INV_HOVER;
    }
    else {
	dispFlags &= ~INV_HOVER;
    }
    dispFlags |= REFRESH_HOVER;
}

void Model::setAuto(bool state)
{
    if (state != autokap) {
	dispFlags |= REFRESH_AUTO_COUNT;
    }
    autokap = state;
    updateLcdShutterState();
}

bool Model::getAuto()
{
    return autokap;
}

void Model::invAuto(bool state)
{
    if (state) {
	dispFlags |= INV_AUTO_COUNT;
    }
    else {
	dispFlags &= ~INV_AUTO_COUNT;
    }
    dispFlags |= REFRESH_AUTO_COUNT;
}

void Model::setDispMode(Mode_t mode)
{
    if (mode != shootMode_disp) {
	shootMode_disp = mode;
	dispFlags |= REFRESH_SHOOT_MODE;
    }
}

void Model::setModeToDispMode()
{
    shootMode = shootMode_disp;
    dispFlags |= REFRESH_SHOOT_MODE;
}

void Model::invMode(bool state)
{
    if (state) {
	dispFlags |= INV_SHOOT_MODE;
    }
    else {
	dispFlags &= ~INV_SHOOT_MODE;
    }
    dispFlags |= REFRESH_SHOOT_MODE;
}

Mode_t Model::getShootMode()
{
    return shootMode;
}

Mode_t Model::getShootModeDisp()
{
    return shootMode_disp;
}

unsigned char Model::getDispFlags()
{
    unsigned char retval = dispFlags;

    dispFlags &= ~CHANGE_FLAGS;

    return retval;
}

unsigned char Model::getLcdShutterState()
{
    return lcdShutterState;
}

bool Model::atGoalPos()
{
    PanTilt_t goal;
    getGoalPwm(&goal);

    return ((goal.pan == servoPos.pan) &&
	    (goal.tilt == servoPos.tilt) &&
	    (servoVel.pan == 0) &&
	    (servoVel.tilt == 0));
}

void Model::getGoalPwm(PanTilt_t *goal)
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

void Model::queueShot(PanTilt_t *aimPoint)
{
    PanTilt_t aimPointPwm;
    toPwm(&aimPointPwm, aimPoint);
    
    queueShotPwm(&aimPointPwm);
}

unsigned Model::getShotsQueued()
{
    return shotsQueued;
}

void Model::dequeueShot()
{
    for (int n = 0; n < SHOT_QUEUE_LEN-2; n++) {
	shotQueue[n] = shotQueue[n+1];
    }
    shotsQueued--;
    dispFlags |= REFRESH_AUTO_COUNT;
}

void Model::getServos(PanTilt_t *pos, PanTilt_t *vel)
{
    *pos = servoPos;
    *vel = servoVel;
}

void Model::setServos(PanTilt_t *pos, PanTilt_t *vel)
{
    servoPos = *pos;
    servoVel = *vel;
}

int Model::getPanPwm()
{
    return servoPos.pan;
}

int Model::getTiltPwm()
{
    return servoPos.tilt;
}

bool Model::getShutter()
{
    return shutterPressed;
}

void Model::setShutter(bool pressed)
{
    shutterPressed = pressed;
}

void Model::setShutterState(unsigned char state)
{
    unsigned char lcd_ss;

    shutterState = state;
    updateLcdShutterState();
}

void Model::setSlewStable(bool state)
{
    slewStable = state;
    updateLcdShutterState();
}

bool Model::getSlewStable()
{
    return slewStable;
}

// --------------------------------------------------------------------------------
// Utility methods

void Model::updateLcdShutterState()
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

void Model::queueShotPwm(PanTilt_t *aimPoint)
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

void Model::toPwm(PanTilt_t *pwm, const PanTilt_t *user)
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

