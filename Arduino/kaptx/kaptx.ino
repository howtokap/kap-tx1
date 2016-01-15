#include <avr/pgmspace.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

#include "Ppm.h"
#include "Joystick.h"
#include "KapTxLcd.h"
#include "KapTxModel.h"
#include "KapTxController.h"

// Pins for Joystick.
#define JS_BUTTON (9)
#define JS_X (A0)
#define JS_Y (A1)

// Create components from libraries
Ppm ppm;
Joystick js(JS_X, JS_Y, JS_BUTTON);
KapTxModel model;                                 // model
KapTxLcd view(&model);         // view 
KapTxController controller(&js, &model);                     // controller

// ----------------------------------------------------------------------------------------------
// PPM for KAP

#define CHAN_PAN (1)
#define CHAN_TILT (2)
#define CHAN_SHUTTER (3)
#define CHAN_HOVER (4)
#define CHAN_UNUSED2 (5)
#define CHAN_UNUSED3 (6)

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
    ppm.write(CHAN_SHUTTER, model.getShutterPwm());
    ppm.write(CHAN_HOVER, model.getHoVerPwm());

    // update LCD
    view.update();
}
