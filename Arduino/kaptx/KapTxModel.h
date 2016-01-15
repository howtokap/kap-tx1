#pragma once

#define SHOT_QUEUE_LEN (36)

enum Mode_e {
    MODE_SINGLE,
    MODE_CLUSTER,
    MODE_VPAN,
    MODE_HPAN,
    MODE_QUAD,
    MODE_360,

    // keep this last
    NUM_MODES,
};
typedef enum Mode_e Mode_t;

enum ShutterState_e {
    SHUTTER_STATE_NO,
    SHUTTER_STATE_RDY,
    SHUTTER_STATE_ACT,
    SHUTTER_STATE_TRIG,

    // keep this last
    NUM_SHUTTER_STATES,
};
typedef enum ShutterState_e ShutterState_t;

// Flags for LCD State
#define REFRESH_PAN_TILT 1
#define REFRESH_AUTO_COUNT 2
#define REFRESH_SHOOT_MODE 4
#define REFRESH_SHUTTER 8
#define REFRESH_HOVER 16
#define CHANGE_FLAGS (REFRESH_PAN_TILT | REFRESH_AUTO_COUNT | REFRESH_SHOOT_MODE | REFRESH_SHUTTER | REFRESH_HOVER)
#define INV_AUTO_COUNT 32
#define INV_SHOOT_MODE 64
#define INV_HOVER 128

#define ANG_360 (24)
#define TILT_MIN (15)
#define TILT_MAX (2)
#define TILT_MID_DEAD (8)
#define PAN_MIN (0)
#define PAN_MAX (23)

struct PanTilt_s {
    int pan;
    int tilt;
};
typedef struct PanTilt_s PanTilt_t;

class KapTxModel
{
  public:
    KapTxModel();

  private:
    // Instance data
    // config params

    // user's selected aim point
    // pan : 0-23 (0 away, increase CW)
    // tilt : -3 - 8 (0 down, 6 level)
    PanTilt_t userPos;

    // current servo positions
    // struct PanTilt servoGoalPos;  // +/- 1000, ms deviation from center PWM.
    PanTilt_t servoPos;      // +/- 1000ms deviation from center PWM
    PanTilt_t servoVel;      // delta pos per 20ms tick

    int shutterServo;

    Mode_t shootMode;
    Mode_t shootMode_disp;

    unsigned int shotsQueued;
    PanTilt_t shotQueue[SHOT_QUEUE_LEN];

    bool hoVer;  // true represents vertical
    bool autokap;

    unsigned char dispFlags;

    unsigned char shutterState;
    unsigned char lcdShutterState;
    bool slewStable;

  public:
    // Public API
    void setPan(int index);
    void adjPan(int increment);
    void setTilt(int index);
    void adjTilt(int increment);
    void getUserPos(PanTilt_s *aimPoint);

    void setHoVer(bool state);
    bool getHoVer();
    void invHoVer(bool state);

    void setAuto(bool state);
    bool getAuto();
    void invAuto(bool state);
    
    void setDispMode(Mode_t mode);
    void setModeToDispMode();
    void invMode(bool state);
    Mode_t getShootMode();
    Mode_t getShootModeDisp();

    // Get display flags (and clear changed flags)
    unsigned char getDispFlags();

    unsigned char getLcdShutterState();

    // Get goal position, as PWM values
    bool atGoalPos();
    void getGoalPwm(PanTilt_t *goal);

    // Queue shot (using indexed pan/tilt values)
    void queueShot(PanTilt_t *aimPoint);
    unsigned getShotsQueued();
    void dequeueShot();
    
    void getServos(PanTilt_t *pos, PanTilt_t *vel);
    void setServos(PanTilt_t *pos, PanTilt_t *vel);
    int getPanPwm();
    int getTiltPwm();
    int getShutterPwm();
    void setShutterPwm(int pwm);
    int getHoVerPwm();

    void setShutterState(unsigned char state);

    void setSlewStable(bool state);
    bool getSlewStable();
    
  private:
    // utility methods
    void updateLcdShutterState();
    void queueShotPwm(PanTilt_t *aimPoint);
    void toPwm(PanTilt_t *pwm, const PanTilt_t *user);
};
