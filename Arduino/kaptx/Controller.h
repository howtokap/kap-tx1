#pragma once 

#include "Joystick.h"
#include "Model.h"



class JsController
{
  public:
    JsController(Joystick *_js, Model *_model);

  private:
    // Instance data
    Joystick *js;
    Model *model;

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
    ShootController(Model *_model);

  private:
    // Instance data
    Model *model;

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
    SlewController(Model *_model);

  private:
    Model *model;

    unsigned char state;
    unsigned int timer;

  public:
    // Public API
    bool update(bool shutterIdle);

  private:
    // Utility methods
    void slew(int *x, int *v, int target, int a);
    void moveServos();
};

// Shutter controller states
#define SHUTTER_IDLE (0)
#define SHUTTER_TRIGGERED (1)
#define SHUTTER_DOWN (2)
#define SHUTTER_POST (3)

class ShutterController
{
  public:
    ShutterController(Model *_model);

  private:
    Model *model;
    unsigned char state;
    unsigned int timer;

  public:
    // Public API
    void update();
    bool isIdle();
};

class Controller
{
  public:
    Controller(Joystick *js, Model *model);

  private:
    // Instance data
    Joystick *js;
    Model *model;

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
