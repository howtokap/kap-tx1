#define SHOT_QUEUE_LEN (36)

struct PanTilt_s {
    int pan;
    int tilt;
};

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
    struct PanTilt_s userPos;

    // current servo positions
    // struct PanTilt servoGoalPos;  // +/- 1000, ms deviation from center PWM.
    struct PanTilt_s servoPos;      // +/- 1000ms deviation from center PWM
    struct PanTilt_s servoVel;      // delta pos per 20ms tick

    int shutterServo;

    unsigned int shootMode;
    unsigned int shootMode_disp;

    unsigned int shotsQueued;
    struct PanTilt_s shotQueue[SHOT_QUEUE_LEN];

    boolean vertical;
    boolean autokap;

  public:
    // Public API
    void setup();
    void update();
    unsigned char getShutterLcdState();

    void setPan(int index);
    void adjPan(int increment);
    void setTilt(int index);
    void adjTilt(int increment);
    void setDispShootMode();
    void setJsMode(int index);  // set shooting mode from index
    void setJsHoVer(int index);
    void setJsAuto(int index);

    // Get goal position, as PWM values
    boolean atGoalPos();
    void getGoalPwm(struct PanTilt_s *goal);

    // Queue shot (using indexed pan/tilt values)
    void queueShot(struct PanTilt_s *aimPoint);
    void dequeueShot();

  private:
    // utility methods
    void queueShotPwm(struct PanTilt_s *aimPoint);
};
