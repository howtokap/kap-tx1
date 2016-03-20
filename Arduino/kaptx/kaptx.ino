#include <avr/pgmspace.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

#include "Ppm.h"
#include "Joystick.h"
#include "View.h"
#include "Model.h"
#include "Controller.h"
#include "Tuning.h"

// Create components from libraries
Ppm ppm;
Joystick js;
Model model;                                 // model
View view(&model);         // view 
Controller controller(&js, &model);                     // controller

// ----------------------------------------------------------------------------------------------
// PPM for KAP

#define CHAN_PAN (1)
#define CHAN_TILT (2)
#define CHAN_SHUTTER (3)
#define CHAN_HOVER (4)
#define CHAN_5_UNUSED (5)
#define CHAN_6_UNUSED (6)

// ---------------------------------------------------------------------

void setup()
{
    Serial.begin(9600);
    Serial.println(F("Hello!"));

    js.setup();
    ppm.setup();
    view.setup();
}

void loop()
{
    // Wait for PPM cycle to start (50Hz)
    ppm.sync();	

    // read inputs
    js.poll();

    // Let controller do it's thing.
    controller.update();

    // update PPM outputs
    struct PanTilt_s pos;
    ppm.write(CHAN_PAN, model.getPanPwm());
    ppm.write(CHAN_TILT, model.getTiltPwm());
    ppm.write(CHAN_SHUTTER, model.getShutter() ? SHUTTER_DOWN_PWM : SHUTTER_UP_PWM);
    ppm.write(CHAN_HOVER, model.getHoVer() ? HOVER_VERT_PWM : HOVER_HOR_PWM);

    // repeat tilt on channel 6 just to test that channel
    ppm.write(CHAN_6_UNUSED, model.getTiltPwm());


    // update LCD
    view.update();
}
