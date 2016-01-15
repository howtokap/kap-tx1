#pragma once 

#include "Joystick.h"
#include "KapTxModel.h"



class JsController
{
  public:
    JsController(Joystick *_js, KapTxModel *_model);

  private:
    // Instance data
    Joystick *js;
    KapTxModel *model;

    // Joystick gesture state machine state
    unsigned char state;

    // start position for slide gesture
    int slideStart_x;
    int slideStart_y;

  public:
    bool update();

  private:
    void setSlideStart();
    bool isSlidingLR();
    bool isSlidingUD();
    void setJsPan();
    void setJsTilt();
    void setJsMode();
    void setJsHoVer();
    void setJsManAuto();
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
    void update(bool jsPressed);

  private:
    // Utility methods
    void shootSingle();
    bool isValidPanTilt(struct PanTilt_s *pt);
    void shootCluster();
    void shootVpan();
    void shootHpan();
    void shootQuad();
    void shoot360();
    int addPan(int pan, int n);
    int addTilt(int tilt, int n);

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
    bool update(bool shutterIdle);

  private:
    // Utility methods
    void slew(int *x, int *v, int target, int a);
    void moveServos();

    // TODO-DW
};

// Shutter controller states
#define SHUTTER_IDLE (0)
#define SHUTTER_TRIGGERED (1)
#define SHUTTER_DOWN (2)
#define SHUTTER_POST (3)

class ShutterController
{
  public:
    ShutterController(KapTxModel *_model);

  private:
    KapTxModel *model;
    unsigned char state;
    unsigned int timer;

  public:
    // Public API
    void update();
    bool isIdle();
    // TODO-DW
};

class KapTxController
{
  public:
    KapTxController(Joystick *js, KapTxModel *model);

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
    void update();

  private:
    // Utility methods
};
