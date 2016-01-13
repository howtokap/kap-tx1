#include <avr/pgmspace.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

#include "Ppm.h"
#include "Joystick.h"
#include "KapTxLcd.h"
#include "KapTxModel.h"

// Pins for Joystick.
#define JS_BUTTON (9)
#define JS_X (A0)
#define JS_Y (A1)

// Pins for LCD module.  (Any pins can be used.)
#define LCD_SCK 4
#define LCD_MOSI 5
#define LCD_SS 6

// Create components from libraries
Ppm ppm;
Joystick js(JS_X, JS_Y, JS_BUTTON);
KapTxModel model;                                 // model
KapTxLcd view(LCD_SCK, LCD_MOSI, LCD_SS);         // view 
KapTxController controller();                     // controller

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

    model.setup();
    view.setup();
    controller.setup();
}

void loop()
{
    // Wait for PPM cycle to start (50Hz)
    ppm.sync();	

    // read inputs
    js.poll();

    // Let controller do it's thing.
    controller.update(&js, &model);

    // Update model
    model.update();

    // update PPM outputs
    ppm.write(CHAN_PAN, model.servoPos.pan);
    ppm.write(CHAN_TILT, model.servoPos.tilt);
    ppm.write(CHAN_SHUTTER, model.shutterServo);
    ppm.write(CHAN_HOVER, model.vertical ? HOVER_VERT_POS : HOVER_HOR_POS);

    // update LCD
    view.update(&model);
}
