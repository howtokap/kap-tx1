#include "Joystick.h"
#include "KapTxModel.h"

class JsController
{
  public:
    JsController(Joystick *_js);

  private:
    // Instance data
    Joystick *js;

    // Joystick gesture state machine state
    unsigned char state;

    // start position for slide gesture
    int slideStart_x;
    int slideStart_y;

  public:
    void update(Joystick *js, KapTxModel *model);

  private:
    void setSlideStart();
    bool isSlidingLR();
    bool isSlidingUD();
};

class ShootController
{
  public:
    ShootController(KapTxModel *_model);

  private:
    // Instance data
    KapTxModel *model;

  public:
    // Public API
    void setup();
    void update();

  private:
    // Utility methods
    void shootSingle();
    boolean isValidPanTilt(struct PanTilt_s *pt);
    void shootCluster();
    void shootVpan();
    void shootHpan();
    void shootQuad();
    void shoot360();
};

class SlewController
{
  public:
    SlewController(KapTxModel *_model);

  private:
    KapTxModel *model;

    unsigned char state;
    unsigned int timer;

  public:
    // Public API
    void setup();
    void update();

  private:
    // Utility methods
    void slew(int *x, int *v, int target, int a);
    void moveServos();

    // TODO-DW
};

class ShutterController
{
  public:
    ShutterController();

  private:
    KapTxModel *model;
    unsigned char state;
    unsigned int timer;

  public:
    // Public API
    void setup();
    void update();

    // TODO-DW
};

class KapTxController
{
  public:
    KapTxController();

  private:
    // Instance data
    Joystick *js;
    KapTxModel *model;

    JsController jsc;
    ShootController shoot;
    SlewController slew;
    ShutterController shutter;

  public:
    // Public methods
    void setup();
    void update(Joystick *js, KapTxModel *model);

  private:
    // Utility methods
};
