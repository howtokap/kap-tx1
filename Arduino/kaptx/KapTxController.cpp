#include "KapTxController.h"

// -------------------------------------------------------------------------------------
// JsController

// Joystick controller states
#define JS_IDLE (0)
#define JS_RIGHT (1)
#define JS_LEFT (2)
#define JS_UP (3)
#define JS_DOWN (4)
#define JS_SET_PAN (5)
#define JS_SET_TILT (6)
#define JS_SET_MODE (7)
#define JS_SET_HOVER (8)
#define JS_SET_AUTO (9)

#define JS_SLIDE_THRESH (100)

JsController::JsController(Joystick *_js, KapTxModel *_model)
{
    // Store references to joystick interface and model
    js = _js;
    model = _model;

    // Init state of this controller
    state = JS_IDLE;
    slideStart_x = 0;
    slideStart_y = 0;
}

bool JsController::update()
{
    switch(state) {
	case JS_IDLE:
	    if (js->isOut()) {
		// int pos = js.getIndex24();
                int pos = js->getIndex16();
                
                switch (pos) {
		    case 0:
		    case 15:
			// joystick is right
			state = JS_RIGHT;
			setSlideStart();
			break;
		    case 1:
		    case 2:
			// joystick is NE, mode select
			state = JS_SET_MODE;
			setSlideStart();
			break;
		    case 3:
		    case 4:
			// joystick is N, tilt 
			state = JS_UP;
			setSlideStart();
			break;
		    case 5:
		    case 6:
                        // joystick is NW, set Auto
                        state = JS_SET_AUTO;
			break;
		    case 7:
		    case 8:
			// joystick is left
			state = JS_LEFT;
			setSlideStart();
			break;
                    case 9:
                    case 10:
                        // future home of config menu
                        break;
		    case 11:
		    case 12:
			// joystick is down
			state = JS_DOWN;
			setSlideStart();
			break;
                    case 13:
                    case 14:
                    	// joystick is SE, HoVer select
			state = JS_SET_HOVER;
                        break;
		    default:
			// joystick is somewhere we don't care about
			break;
		}
	    }
	    break;
	case JS_RIGHT:
	    if (js->isCenter()) {
		// was a bump right
		model->adjPan(-1);
		state = JS_IDLE;
	    }
	    if (isSlidingUD()) {
		// Start setting Pan from Joystick
		state = JS_SET_PAN;
		setJsPan()
	    }
	    break;
	case JS_LEFT:
	    if (js->isCenter()) {
		// was a bump left
		adjPan(1);
		state = JS_IDLE;
	    }
	    if (isSlidingUD()) {
		// Start tracking pan with joystick
		state = JS_SET_PAN;
		setJsPan();
	    }
	    break;
	case JS_UP:
	    if (js->isCenter()) {
		// was a bump up
		adjTilt(1);
		state = JS_IDLE;
	    }
	    if (isSlidingLR()) {
		// Start tracking tilt with joystick
		state = JS_SET_TILT;
		setJsTilt();
	    }
	    break;
	case JS_DOWN:
	    if (js->isCenter()) {
		// was a bump down
		adjTilt(-1);
		state = JS_IDLE;
	    }
	    if (isSlidingLR()) {
		state = JS_SET_TILT;
		setJsTilt();
	    }
	    break;
	case JS_SET_PAN:
	    if (js->isCenter()) {
		// JS Pan is done
		state = JS_IDLE;
	    }
	    else {
		// Continue tracking joystick pan
		setJsPan();
	    }
	    break;
	case JS_SET_TILT:
	    if (js->isCenter()) {
		// JS Tilt is done
		state = JS_IDLE;
	    }
	    else {
		// Continue tracking joystick tilt
		setJsTilt();
	    }
	    break;
	case JS_SET_MODE:
	    if (js->isCenter()) {
		// set new mode
		model->setDispShootMode();
		// JS Mode is done
		state = JS_IDLE;
	    }
	    else {
		// Continue tracking joystick mode
		setJsMode();
	    }
	    break;
	case JS_SET_HOVER:
	    if (js->isCenter()) {
		state = JS_IDLE;
	    }
	    else {
		// Track HOVER setting
                setJsHover();
	    }
	    break;
        case JS_SET_AUTO:
            if (js->isCenter()) {
                state = JS_IDLE;
            }
            else {
                // Track AUTO setting
		setJsManAuto();
            }
            break;
	default:
	    // bad state -- fix it
	    state = JS_IDLE;
	    break;
    }

    // Return indicator that joystick was pressed to trigger shoot controller.
    // (TODO-DW : Produce this within state machine instead of passing raw value from JS.)
    return js->wasPressed();
}

void JsController::setJsManAuto()
{
    int i = js->getIndex16();

    if ((i == 3) || (i == 4)) {
	// set in auto mode
	model->autokap = true;
    }
    if ((i == 7) || (i == 8)) {
	// turn off auto mode
	model->autokap = false;
    }
}

void JsController::setJsHoVer()
{
    int i = js->getIndex16();

    if ((i == 0) || (i == 7) || (i == 8) || (i == 15)) {
	// set in horizontal mode
	model->vertical = false;
    }
    if ((i == 3) || (i == 4) || (i == 11) || (i == 12)) {
	// set in vertical mode
	model->vertical = true;
    }
}

void JsController::setJsPan()
{
    int index = js.getIndex24();

    model.userPos.pan = index;
}

void JsController::setJsTilt()
{
    int index = js.getIndex24();
    
    // Note: TILT_MAX is at index 2, TILT_MIN at 15.
    // This is because tilt indices use standard orientation.
    if ((index <= TILT_MAX) || (index >= TILT_MIN)) {
      // adopt this tilt
      model.userPos.tilt = index;
    }
    else if (index < (TILT_MIN + TILT_MAX)/2) {
        // set max tilt
        model.userPos.tilt = TILT_MAX;
    }
    else {
        // set min tilt
        model.userPos.tilt = TILT_MIN;
    }
}

void JsController::setJsMode()
{
    int mode;
    
    switch (js.getIndex24()) {
      case 3:
        mode = MODE_SINGLE;
        break;
      case 1:
        mode = MODE_CLUSTER;
        break;
      case 23:
        mode = MODE_VPAN;
        break;
      case 21:
        mode = MODE_HPAN;
        break;
      case 5:
        mode = MODE_QUAD;
        break;
      case 7:
        mode = MODE_360;
        break;
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
        // show current shoot mode
        mode = model->shootMode;
        break;
      default:
        // otherwise, don't change shootMode_disp
        mode = model->shootMode_disp;
    }
    
    // set display mode
    model->setDispMode(mode);
}

void JsController::setSlideStart()
{
    slideStart_x = x;
    slideStart_y = y;
}

bool JsController::isSlidingLR()
{
    int move = iabs(x - slideStart_x);
    return (move > JS_SLIDE_THRESH);
}

bool JsController::isSlidingUD()
{
    int move = iabs(y - slideStart_y);
    return (move > JS_SLIDE_THRESH);
}

// -------------------------------------------------------------------------------------
// ShootController

ShootController::ShootController(KapTxModel *_model)
{
    model = _model;
    js = _js;
}

void ShootController::setup()
{
}



void ShootController::update(bool jsPressed)
{
  bool trigger = false;
  
  if (model->autokap) {
    // trigger new sequence when prior one finishes
    if (model->shotsQueued == 0) {
      trigger = true;
    }
  }
  else {
    // trigger new sequence when operator presses joystick
    if (jsPressed()) {
      trigger = true;
    }
  }

  if (trigger) {
    // perform shot processing based on mode
    switch (model->shootMode) {
	case MODE_SINGLE:
	    shootSingle();
	    break;
	case MODE_CLUSTER:
	    shootCluster();
	    break;
	case MODE_VPAN:
	    shootVpan();
	    break;
	case MODE_HPAN:
	    shootHpan();
	    break;
	case MODE_QUAD:
	    shootQuad();
	    break;
	case MODE_360:
	    shoot360();
	    break;
	default:
	    // reset to single shoot mode
            model->shootMode = MODE_SINGLE;
	    break;
    }
  }
}

void ShootController::shootSingle()
{
  // queue a shot based on current pan/tilt
  queueShot(&model.userPos);
}

boolean ShootController::isValidPanTilt(struct PanTilt_s *pt)
{
    // comparison looks wrong but it's actually right.
    // TILT_MAX = 2, TILT_MIN = 15.  Unreachable zone is 3-14.
    if ((pt->tilt > TILT_MAX) && (pt->tilt < TILT_MIN)) {
	return false;
    }

    return true;
}

#define SEQ_LEN 7

void ShootController::shootCluster()
{
	// queue a sequence of shots
	struct PanTilt_s aimPointBase;
        struct PanTilt_s aimPoint;

	// queue a series of shots.
	static const struct PanTilt_s seq_high[SEQ_LEN] PROGMEM = {
	    {0, 0}, {1, 1}, {2, 0}, {1, -1}, {-1, -1}, {-2, 0}, {-1, 1},
	};
	static const struct PanTilt_s seq_med[SEQ_LEN] PROGMEM = {
	    {0, 0}, {1, 1}, {2, 0}, {2, -1}, {-2, -1}, {-2, 0}, {-1, 1},
	};
	static const struct PanTilt_s seq_low[SEQ_LEN] PROGMEM = {
	    {0, 0}, {0, 2}, {0, -2}, {4, -2}, {4, 2}, {-4, 2}, {-4, -2},
	};
        const struct PanTilt_s *pSeq;
        
        // base aim point is user's aim point initially
        aimPointBase = model.userPos;
        
        // get tilt off the rails so cluster doesn't collapse on itself.
        if (aimPointBase.tilt == TILT_MAX) aimPointBase.tilt -= 1;
        if (aimPointBase.tilt == TILT_MIN) aimPointBase.tilt += 1;
        
        // Choose sequence
        if (aimPointBase.tilt <= 2) {
            pSeq = seq_high;
        }
        else if (aimPointBase.tilt >= 22) {
            pSeq = seq_high;
        }
        else if (aimPointBase.tilt >= 19) {
            pSeq = seq_med;
        }
        else if (aimPointBase.tilt == 18) {
            pSeq = seq_low;
        }
        else {
            // use medium cluster pattern, with pan+180, tilt on other side of zenith
            pSeq = seq_med;
            aimPointBase.pan = aimPointBase.pan + 12;
            if (aimPointBase.pan >= 24) aimPointBase.pan -= 24;
            aimPointBase.tilt = 18 + (18 - aimPointBase.tilt);
        }

	for (int n = 0; n < SEQ_LEN; n++) {
            PanTilt_s offset;
            memcpy_P(&offset, pSeq+n, sizeof(PanTilt_s));
            
            aimPoint = aimPointBase;
            aimPoint.pan = addPan(aimPoint.pan, offset.pan);
	    aimPoint.tilt = addTilt(aimPoint.tilt, offset.tilt);

            // sprintf(s, "queue shot %d, %d.", aimPoint.pan, aimPoint.tilt);
            // Serial.println(s);
            queueShot(&aimPoint);
	}
}

void ShootController::shootVpan()
{
  struct PanTilt_s aimPoint;
  int tilt;
  
    // queue up the userPos position for first shot.
    queueShot(&model.userPos);
    
    // Try to shoot +/- 45 degrees of tilt around user's tilt
    // Compute highest tilt
    tilt = model.userPos.tilt;
    
    // adjust down by 45 degrees (constrained by range of motion)
    tilt -= 3;  
    if (tilt < 0) tilt += 23;
    if (tilt < 15) tilt = 15;
    
    // adjust up by 90 degrees (constrained by range of motion)
    tilt += 6;
    if (tilt >= 24) tilt -= 24;
    if ((tilt > TILT_MAX) && (tilt < TILT_MIN)) tilt = TILT_MAX;
   
    // Take shots spanning 90 degrees
    for (int n = 0; n < 7; n++) {
      aimPoint.pan = model.userPos.pan;
      aimPoint.tilt = tilt - n;
      if (aimPoint.tilt < 0) aimPoint.tilt += 24;
      queueShot(&aimPoint);
    }
}

void ShootController::shootHpan()
{
  struct PanTilt_s aimPoint;
  int tilt = model.userPos.tilt;
  int tilt2 = ((tilt <= 2) || (tilt > 18)) ? tilt-2 : tilt + 2;
  
  if (tilt2 < 0) tilt2 += 24;
  
    // queue up the userPos position for first shot.
    aimPoint = model.userPos;
    queueShot(&aimPoint);
       
    // Take shots 45 degrees left to 45 right, two rows
    for (int n = -3; n <= 3; n += 2) {
      aimPoint.pan = model.userPos.pan + n;
      if (aimPoint.pan < 0) aimPoint.pan += 24;
      if (aimPoint.pan >= 24) aimPoint.pan -= 24;
      aimPoint.tilt = tilt;
      queueShot(&aimPoint);
      
      aimPoint.tilt = tilt2;
      queueShot(&aimPoint);
    }
}

void ShootController::shootQuad()
{
  static const struct PanTilt_s quadSeq[] PROGMEM = {
    {-3, 0}, {-3, -2}, {-3, -4}, {-3, -6},
    {-1, 0}, {-1, -2},
    { 0, -4},
    { 1, -2}, { 1, 0}, 
    { 3, 0}, {3, -2}, {3, -4}, {3, -6},
  };
  
  struct PanTilt_s reference;
  struct PanTilt_s aimPoint;
  
    reference = model.userPos;
    reference.tilt = 0;
    for (int n = 0; n < sizeof(quadSeq)/sizeof(quadSeq[0]); n++) {
      PanTilt_s offset;
      memcpy_P(&offset, quadSeq+n, sizeof(PanTilt_s));
      aimPoint = reference;
      aimPoint.pan = addPan(aimPoint.pan, offset.pan);
      aimPoint.tilt = addTilt(aimPoint.tilt, offset.tilt);
      queueShot(&aimPoint);
    }
}

void ShootController::shoot360()
{
  static const struct PanTilt_s seq360[] PROGMEM = {
    { 0, 0}, { 0, -2}, { 0, -4}, { 0, -6},
    { 2, 0}, { 2, -2},
    { 3, -4},
    { 4, -2}, { 4, 0}, 
  };
  
  struct PanTilt_s reference;
  struct PanTilt_s aimPoint;
  
    for (int i = 0; i < 4; i++) {   // shoot 4 quadrants
      reference = model.userPos;
      reference.tilt = 0;
      reference.pan = addPan(model.userPos.pan, i*6);
      for (int n = 0; n < sizeof(seq360)/sizeof(seq360[0]); n++) {
        PanTilt_s offset;
        memcpy_P(&offset, seq360+n, sizeof(PanTilt_s));
        aimPoint = reference;
        aimPoint.pan = addPan(aimPoint.pan, offset.pan);
        aimPoint.tilt = addTilt(aimPoint.tilt, offset.tilt);
        queueShot(&aimPoint);
      }
    }
}

int ShootController::addPan(int pan, int n)
{
  pan += n;
  if (pan < 0) pan += ANG_360;
  if (pan >= ANG_360) pan -= ANG_360;
  
  return pan;
}

int ShootController::addTilt(int tilt, int n)
{
  tilt += n;
  if (tilt >= ANG_360) tilt -= ANG_360;
  if (tilt < 0) tilt += ANG_360;
  
  if ((tilt > TILT_MAX) && (tilt < TILT_MID_DEAD)) tilt = TILT_MAX;
  if ((tilt < TILT_MIN) && (tilt >= TILT_MID_DEAD)) tilt = TILT_MIN;
  
  return tilt;
}

// -------------------------------------------------------------------------------------
// SlewController

SlewController::SlewController(KapTxModel *_model)
{
    model = _model;
    state = SLEW_STABLE:
    timer = 0;
}

void SlewController::update()
{
    // execute slew state machine to bring servos to target position.
    switch (state) {
	case SLEW_STABLE:
	    // Serial.println("slew state STABLE");
	    if ((model.shutterState == SHUTTER_IDLE) &&
		(!atGoalPos())) {
		// start moving
		state = SLEW_MOVING;
		moveServos();
	    }
	    break;
	case SLEW_MOVING:
	    // Serial.println("slew state MOVING");
	    if (!atGoalPos()) {
		moveServos();
	    } else {
		timer = TIME_STABILIZING;
		state = SLEW_STABILIZING;
	    }
	    break;
	case SLEW_STABILIZING:
	    // Serial.println("slew state STABILIZING");
	    if (!atGoalPos()) {
		timer = 0;
		state = SLEW_MOVING;
		moveServos();
	    }
	    else if (timer > 0) {
		// Wait for slew stabilization time
		timer--;
	    } else {
		// waiting is done, we're stable
		state = SLEW_STABLE;
	    }
	    break;
	default:
	    // Serial.println("slew state default!");
	    // This should not have happened!
	    state = SLEW_STABLE;
	    break;
    }
}

void SlewController::slew(int *x, int *v, int target, int a)
{
    // compute position error
    int deltax = target - *x;
    int norm_v = *v;
    boolean reverse = false;
    if (deltax < 0) {
	reverse = true;
	deltax = -deltax;
	norm_v = -norm_v;
    }

    // compute limiting velocity
    int v_limit = 0;
    int dist = 0;
    while (dist < deltax) {
	v_limit += a;
	dist += v_limit;
    }

    if ((deltax <= a) && (norm_v <= a) && (norm_v >= -a)) {
	// hit the target and stop
	*x = target;
	*v = 0;
    } 
    else {
	if (norm_v + a < v_limit) {
	    // accelerate
	    norm_v += a;
	}
	else {
	    // decelerate
	    norm_v -= a;
	}
	if (!reverse) {
	    *x += norm_v;
	    *v = norm_v;
	} 
	else {
	    *x += -norm_v;
	    *v = -norm_v;
	}
    }
}

void SlewController::moveServos()
{
    // update servo target position from userPos
    struct PanTilt_s goal;

    model->getGoalPwm(&goal);

    // TODO-DW : Keep pos, vel in controller, set pos in model as output
    // TODO-DW : Incorporate HoVer servo into slew controller

    slew(&model.servoPos.pan, &model.servoVel.pan, goal.pan, ACCEL_PAN);
    slew(&model.servoPos.tilt, &model.servoVel.tilt, goal.tilt, ACCEL_TILT);

}

// -------------------------------------------------------------------------------------
// ShutterController

ShutterController::ShutterController()
{
}

void ShutterController::update(bool slewStable)
{
    switch (state) {
	case SHUTTER_IDLE:
	    // Serial.println("shutter state IDLE");
	    if (model->shotsQueued != 0) {
		// transition to TRIGGERED state
		state = SHUTTER_TRIGGERED;
	    }
	    break;
	case SHUTTER_TRIGGERED:
	    // Serial.println("shutter state TRIGGERED");
	    if (slewStable) {
		// trip shutter and transition to DOWN state
		model.shutterServo = SHUTTER_DOWN_POS;
		timer = TIME_SHUTTER_DOWN;
		state = SHUTTER_DOWN;
		// Serial.println("shutter state DOWN");
	    }
	    break;
	case SHUTTER_DOWN:
	    // Serial.println("shutter state DOWN");
	    if (timer > 0) {
		timer--;
	    } else {
		// shutter has been down long enough
		model.shutterServo = SHUTTER_UP_POS;
		timer = TIME_SHUTTER_POST;
		state = SHUTTER_POST;
		// Serial.println("shutter state POST");
	    }
	    break;
	case SHUTTER_POST:
	    // Serial.println("shutter state POST");
	    if (timer > 0) {
		timer--;
	    } else {
		// shutter has been up long enough
		model->dequeueShot();
		state = SHUTTER_IDLE;
		// Serial.println("shutter state IDLE");
	    }
	    break;
	default:
	    // Serial.println("Bogus shutter state!");
	    state = SHUTTER_IDLE;
	    break;
    }
}

// -------------------------------------------------------------------------------------
// KapTxController

KapTxController::KapTxController(Joystick *js, KapTxModel *model) :
    jsc(js, model),
    shoot(model),
    slew(model),
    shutter(model)
{
    // TODO
}

void KapTxController::setup()
{
    // Setup each controller component
    jsc.setup();
    shoot.setup();
    slew.setup();
    shutter.setup();
}

// Controller actions
void KapTxController::update()
{
    bool slewStable;

    // Update each controller component
    jsPressed = jsc.update();  // TODO-DW : Better way to communicate joystick presses to shoot controller?
    shoot.update(jsPressed);
    slewStable = slew.update();
    shutter.update(slewStable);
}
