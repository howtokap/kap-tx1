#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <avr/pgmspace.h>

// Pan and Tilt Servo tuning
// The controller software represents angles with integers from 0 to 23.
// 0 represents 0 degrees (pan to the right, tilt is horizontal)
// 6 represents 90 degrees, pan downwind, tilt would be up (not allowed)
// 12 represents 180 degrees, pan left, tilt horizontal but backwards (invalid)
// 18 represents 270 degrees, pan upwind, tilt straight down.
// [Note: valid tilt angles are +30 degrees to -135
//        in this scheme, these correspond to 2, 1, 0, 23, 22, ..., 15.]
//
// To move the servos to the corresponding positions, these need to be converted
// to PWM offsets in the range -1600 to 1600.  (Units are microseconds offset
// from center position)
// The formulas used are: 
//     PAN PWM = <pan> * PWM_FACTOR_PAN + PWM_OFFSET_PAN.
//     TILT PWM = <tilt> * PWM_FACTOR_TILT + PWM_OFFSET_TILT.
//
// The values provided below are correct for my rig but probably not yours.
// You will need to change them.
//
// To find the correct values, try this procedure:
// Step 1 : Determine offsets.
//      Set PWM_FACTOR_PAN and PWM_FACTOR_TILT to 0.
//      Run the software and see where the servos aim just after power on.
//      Then, modify PWM_OFFSET_PAN and PWM_OFFSET_TILT and retry this.
//      Keep adjusting these until the tilt is horizontal and the pan is to
//           the right.
//      (The offsets should stay in the range -1600 to 1600)
//
// Step 2 : Determine factors.
//      Now set the PWM_FACTOR_PAN and PWM_FACTOR_TILT parameters to 100.
//      Run the software, then adjust aim point on the display to be 
//           pan straight away from you, tilt straight down.
//      Is the rig pointing there?  No, adjust the factors and try again.
//      If the rig moves the wrong way, the factor will need to change sign.
//      
// Once the factors are correct, the pan and tilt of the rig should 
// match the controller's display for all angles.
//      

#define PWM_FACTOR_PAN (133)
#define PWM_OFFSET_PAN (-800)
#define PWM_FACTOR_TILT (265)
#define PWM_OFFSET_TILT (893)
#define PWM_MAX_OFFSET (1600)

// for debug prints
char s[40];

// Pins for LCD module.  (Any pins can be used.)
#define LCD_SCK 4
#define LCD_MOSI 5
#define LCD_SS 6

Adafruit_SharpMem display(LCD_SCK, LCD_MOSI, LCD_SS);

#define BLACK 0
#define WHITE 1

// Ids for bitmaps
enum {
    BM_NONE,     // To support nop in showBitmap()
    BM_SHOOT_NO,
    BM_SHOOT_RDY,
    BM_SHOOT_ACT,
    BM_SHOOT_TRIG,
    BM_MODE_SINGLE,
    BM_MODE_CLUSTER,
    BM_MODE_VPAN,
    BM_MODE_HPAN,
    BM_MODE_QUAD,
    BM_MODE_360,
    BM_CIRCLE,
    BM_ARC_0,
    BM_ARC_1,
    BM_ARC_2,
    BM_ARC_15,
    BM_ARC_16,
    BM_ARC_17,
    BM_ARC_18,
    BM_ARC_19,
    BM_ARC_20,
    BM_ARC_21,
    BM_ARC_22,
    BM_ARC_23,
};

struct xbm_s {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    const unsigned char *bits;
};

#define PAN_TILT_X_OFFSET 0
#define PAN_TILT_Y_OFFSET 32

#define circ_x (0 + PAN_TILT_X_OFFSET)
#define circ_y (0 + PAN_TILT_Y_OFFSET)
#define circ_width 64
#define circ_height 64
static const unsigned char circ_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x87,
    0xe1, 0x03, 0x00, 0x00, 0x00, 0x00, 0x78, 0x80, 0x01, 0x1e, 0x00, 0x00,
    0x00, 0x00, 0x0e, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00,
    0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x18, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x0f, 0x00, 0x00,
    0x00, 0x00, 0xf0, 0x00, 0x80, 0x0d, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x01,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x08, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x30, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x60, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x60, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x0d, 0x00, 0x00,
    0x00, 0x00, 0xb0, 0x01, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x0c, 0x00, 0x00,
    0x00, 0x00, 0x30, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00,
    0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0xc0, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00,
    0x00, 0x00, 0x0e, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x78, 0x80,
    0x01, 0x1e, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x87, 0xe1, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00 };

#define arc_0_x (53 + PAN_TILT_X_OFFSET)
#define arc_0_y (21 + PAN_TILT_Y_OFFSET)
#define arc_0_width 8
#define arc_0_height 22
static const unsigned char arc_0_bits[] PROGMEM = {
    0x30, 0x3e, 0x7f, 0x7e, 0x7e, 0x7e, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0x7e, 0x7e, 0x7e, 0x7f, 0x3c, 0x20 };

#define arc_1_x (50 + PAN_TILT_X_OFFSET)
#define arc_1_y (14 + PAN_TILT_Y_OFFSET)
#define arc_1_width 11
#define arc_1_height 21
static const unsigned char arc_1_bits[] PROGMEM = {
    0x10, 0x00, 0x38, 0x00, 0x7c, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0xfe, 0x00,
    0xfc, 0x01, 0xfc, 0x01, 0xf8, 0x01, 0xf8, 0x03, 0xf0, 0x03, 0xf0, 0x03,
    0xf0, 0x03, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07,
    0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07 };

#define arc_2_x (46 + PAN_TILT_X_OFFSET)  // x, y in doubt
#define arc_2_y (9 + PAN_TILT_Y_OFFSET)
#define arc_2_width 15
#define arc_2_height 20
static const unsigned char arc_2_bits[] PROGMEM = {
    0x08, 0x00, 0x1c, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0xff, 0x00, 0xfe, 0x01,
    0xfc, 0x03, 0xf8, 0x07, 0xf0, 0x07, 0xe0, 0x0f, 0xe0, 0x0f, 0xc0, 0x1f,
    0xc0, 0x1f, 0x80, 0x1f, 0x80, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f,
    0x00, 0x7e, 0x00, 0x06 };

#define arc_15_x (5 + PAN_TILT_X_OFFSET)
#define arc_15_y (41 + PAN_TILT_Y_OFFSET)
#define arc_15_width 18
#define arc_15_height 18
static const unsigned char arc_15_bits[] PROGMEM = {
    0x30, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x00,
    0xfe, 0x00, 0x00, 0xfc, 0x01, 0x00, 0xfc, 0x03, 0x00, 0xf8, 0x07, 0x00,
    0xf0, 0x0f, 0x00, 0xe0, 0x1f, 0x00, 0xc0, 0x7f, 0x00, 0xc0, 0xff, 0x01,
    0x00, 0xff, 0x03, 0x00, 0xfe, 0x03, 0x00, 0xfc, 0x03, 0x00, 0xf8, 0x01,
    0x00, 0xe0, 0x01, 0x00, 0x80, 0x00 };

#define arc_16_x (10 + PAN_TILT_X_OFFSET)
#define arc_16_y (46 + PAN_TILT_Y_OFFSET)
#define arc_16_width 19
#define arc_16_height 15
static const unsigned char arc_16_bits[] PROGMEM = {
    0x08, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x7f, 0x00, 0x00,
    0xff, 0x00, 0x00, 0xfe, 0x03, 0x00, 0xfe, 0x0f, 0x00, 0xf8, 0x3f, 0x00,
    0xf0, 0xff, 0x01, 0xe0, 0xff, 0x07, 0xc0, 0xff, 0x07, 0x00, 0xff, 0x07,
    0x00, 0xfc, 0x07, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x02 };

#define arc_17_x (15 + PAN_TILT_X_OFFSET)
#define arc_17_y (51 + PAN_TILT_Y_OFFSET)
#define arc_17_width 21
#define arc_17_height 10
static const unsigned char arc_17_bits[] PROGMEM = {
    0x1c, 0x00, 0x00, 0x7e, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xff, 0x0f, 0x00,
    0xff, 0xff, 0x0f, 0xfe, 0xff, 0x1f, 0xf8, 0xff, 0x1f, 0xe0, 0xff, 0x1f,
    0x00, 0xff, 0x1f, 0x00, 0xf0, 0x1f };

#define arc_18_x (21 + PAN_TILT_X_OFFSET)
#define arc_18_y (53 + PAN_TILT_Y_OFFSET)
#define arc_18_width 22
#define arc_18_height 8
static const unsigned char arc_18_bits[] PROGMEM = {
    0x04, 0x00, 0x08, 0x3c, 0x00, 0x1f, 0xfe, 0xff, 0x1f, 0xfe, 0xff, 0x1f,
    0xfe, 0xff, 0x3f, 0xff, 0xff, 0x3f, 0xfc, 0xff, 0x0f, 0xc0, 0xff, 0x00 };

#define arc_19_x (29 + PAN_TILT_X_OFFSET)
#define arc_19_y (51 + PAN_TILT_Y_OFFSET)
#define arc_19_width 21
#define arc_19_height 11
static const unsigned char arc_19_bits[] PROGMEM = {
    0x00, 0x00, 0x02, 0x00, 0x80, 0x03, 0x00, 0xe0, 0x07, 0x00, 0xf8, 0x0f,
    0x00, 0xff, 0x1f, 0xff, 0xff, 0x0f, 0xff, 0xff, 0x07, 0xff, 0xff, 0x01,
    0xff, 0x7f, 0x00, 0xff, 0x0f, 0x00, 0xff, 0x00, 0x00 };

#define arc_20_x (35 + PAN_TILT_X_OFFSET)
#define arc_20_y (46 + PAN_TILT_Y_OFFSET)
#define arc_20_width 20
#define arc_20_height 15
static const unsigned char arc_20_bits[] PROGMEM = {
    0x00, 0x80, 0x00, 0x00, 0xc0, 0x03, 0x00, 0xe0, 0x07, 0x00, 0xf0, 0x0f,
    0x00, 0xf8, 0x07, 0x00, 0xfe, 0x03, 0x80, 0xff, 0x03, 0xe0, 0xff, 0x00,
    0xfc, 0x7f, 0x00, 0xff, 0x3f, 0x00, 0xff, 0x1f, 0x00, 0xfe, 0x07, 0x00,
    0xfe, 0x01, 0x00, 0x3e, 0x00, 0x00, 0x02, 0x00, 0x00 };

#define arc_21_x (41 + PAN_TILT_X_OFFSET)
#define arc_21_y (40 + PAN_TILT_Y_OFFSET)
#define arc_21_width 18
#define arc_21_height 18
static const unsigned char arc_21_bits[] PROGMEM = {
    0x00, 0x70, 0x00, 0x00, 0xf8, 0x01, 0x00, 0xf8, 0x03, 0x00, 0xfc, 0x01,
    0x00, 0xfc, 0x01, 0x00, 0xfe, 0x00, 0x00, 0xff, 0x00, 0x80, 0x7f, 0x00,
    0xc0, 0x3f, 0x00, 0xe0, 0x1f, 0x00, 0xf8, 0x0f, 0x00, 0xfe, 0x0f, 0x00,
    0xff, 0x03, 0x00, 0xff, 0x01, 0x00, 0xfe, 0x00, 0x00, 0x7e, 0x00, 0x00,
    0x1c, 0x00, 0x00, 0x04, 0x00, 0x00 };

#define arc_22_x (46 + PAN_TILT_X_OFFSET)
#define arc_22_y (35 + PAN_TILT_Y_OFFSET)
#define arc_22_width 15
#define arc_22_height 19
static const unsigned char arc_22_bits[] PROGMEM = {
    0x00, 0x3e, 0x00, 0x7e, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x3f, 0x80, 0x3f,
    0x80, 0x1f, 0xc0, 0x1f, 0xc0, 0x1f, 0xe0, 0x0f, 0xe0, 0x0f, 0xf0, 0x07,
    0xf8, 0x07, 0xfc, 0x03, 0xfe, 0x01, 0xff, 0x00, 0x7e, 0x00, 0x7c, 0x00,
    0x18, 0x00 };

#define arc_23_x (51 + PAN_TILT_X_OFFSET)
#define arc_23_y (28 + PAN_TILT_Y_OFFSET)
#define arc_23_width 10
#define arc_23_height 21
static const unsigned char arc_23_bits[] PROGMEM = {
    0xe0, 0x03, 0xf0, 0x03, 0xf0, 0x03, 0xf0, 0x03, 0xf0, 0x03, 0xf0, 0x03,
    0xf0, 0x03, 0xf0, 0x03, 0xf0, 0x03, 0xf8, 0x01, 0xf8, 0x01, 0xf8, 0x01,
    0xfc, 0x01, 0xfc, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x7f, 0x00,
    0x3f, 0x00, 0x3e, 0x00, 0x18, 0x00 };

#define MODE_OFFSET_X 64
#define MODE_OFFSET_Y 0

#define bm_single_x MODE_OFFSET_X
#define bm_single_y MODE_OFFSET_Y
#define bm_single_width 32
#define bm_single_height 32
static const unsigned char bm_single_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xf0, 0x1f, 0x00,
    0x00, 0xf8, 0x3f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00,
    0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00,
    0x00, 0xfc, 0x7f, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0xf8, 0x3f, 0x00,
    0x00, 0xf0, 0x1f, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define bm_cluster_x MODE_OFFSET_X
#define bm_cluster_y MODE_OFFSET_Y
#define bm_cluster_width 32
#define bm_cluster_height 32
static const unsigned char bm_cluster_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x80, 0x3f, 0xfc, 0x01,
    0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x7f, 0xfe, 0x03,
    0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x7f, 0xfe, 0x03, 0x80, 0x3f, 0xfc, 0x01,
    0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf8, 0xc0, 0x07, 0x3e, 0xfc, 0xe1, 0x0f, 0x7f, 0xfe, 0xf3, 0x9f, 0xff,
    0xfe, 0xf3, 0x9f, 0xff, 0xfe, 0xf3, 0x9f, 0xff, 0xfe, 0xf3, 0x9f, 0xff,
    0xfe, 0xf3, 0x9f, 0xff, 0xfc, 0xe1, 0x0f, 0x7f, 0xf8, 0xc0, 0x07, 0x3e,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00,
    0x80, 0x3f, 0xfc, 0x01, 0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x7f, 0xfe, 0x03,
    0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x7f, 0xfe, 0x03,
    0x80, 0x3f, 0xfc, 0x01, 0x00, 0x1f, 0xf8, 0x00
};

#define bm_vpan_x MODE_OFFSET_X
#define bm_vpan_y MODE_OFFSET_Y
#define bm_vpan_width 32
#define bm_vpan_height 32
static const unsigned char bm_vpan_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
    0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xf8, 0x0f, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
    0x00, 0xf0, 0x07, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf8, 0x0f, 0x00,
    0x00, 0xf0, 0x07, 0x00, 0x00, 0xe0, 0x03, 0x00
};

#define bm_hpan_x 63
#define bm_hpan_y 1
#define bm_hpan_width 32
#define bm_hpan_height 32
static const unsigned char bm_hpan_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf8, 0xc0, 0x07, 0x3e, 0xfc, 0xe1, 0x0f, 0x7f, 0xfe, 0xf3, 0x9f, 0xff,
    0xfe, 0xf3, 0x9f, 0xff, 0xfe, 0xf3, 0x9f, 0xff, 0xfe, 0xf3, 0x9f, 0xff,
    0xfe, 0xf3, 0x9f, 0xff, 0xfc, 0xe1, 0x0f, 0x7f, 0xf8, 0xc0, 0x07, 0x3e,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define bm_quad_x 63
#define bm_quad_y 1
#define bm_quad_width 32
#define bm_quad_height 32
static const unsigned char bm_quad_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00, 0x00, 0xf8, 0x03, 0x00, 0x00,
    0xf8, 0x03, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00,
    0xf8, 0xe3, 0x00, 0x00, 0xf0, 0x81, 0x03, 0x00, 0x40, 0x00, 0x7e, 0x00,
    0x40, 0x00, 0xfe, 0x00, 0x40, 0x00, 0xfe, 0x00, 0x40, 0x00, 0xfe, 0x00,
    0xf0, 0x01, 0xfe, 0x00, 0xf8, 0x03, 0xfe, 0x00, 0xf8, 0x03, 0xfc, 0x00,
    0xf8, 0x03, 0x80, 0x01, 0xf8, 0x03, 0x00, 0x01, 0xf8, 0x03, 0x00, 0x03,
    0xf0, 0x01, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x06,
    0x40, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00, 0x04, 0xf0, 0x81, 0x0f, 0x3e,
    0xf8, 0xc3, 0x1f, 0x7f, 0xf8, 0xc3, 0x1f, 0x7f, 0xf8, 0xff, 0xff, 0x7f,
    0xf8, 0xc3, 0x1f, 0x7f, 0xf8, 0xc3, 0x1f, 0x7f, 0xf0, 0x81, 0x0f, 0x3e,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define bm_360_x 63
#define bm_360_y 1
#define bm_360_width 32
#define bm_360_height 32
static const unsigned char bm_360_bits[] PROGMEM = {
    0x00, 0xf0, 0x1f, 0x00, 0x00, 0x1e, 0xf0, 0x00, 0x00, 0x8f, 0xe3, 0x01,
    0xc0, 0x8f, 0xe3, 0x07, 0x60, 0x8e, 0xe3, 0x0c, 0x30, 0x00, 0x00, 0x18,
    0x10, 0x00, 0x00, 0x10, 0x18, 0x00, 0x00, 0x30, 0xcc, 0x71, 0x1c, 0x67,
    0xc4, 0x71, 0x1c, 0x47, 0xc4, 0x71, 0x1c, 0x47, 0x06, 0x00, 0x00, 0xc0,
    0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x3a, 0x8e, 0xe3, 0xb8,
    0x3a, 0x8e, 0xe3, 0xb8, 0x3a, 0x8e, 0xe3, 0xb8, 0x02, 0x00, 0x00, 0x80,
    0x02, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0xc0, 0xc4, 0x71, 0x1c, 0x47,
    0xc4, 0x71, 0x1c, 0x47, 0xcc, 0x71, 0x1c, 0x67, 0x18, 0x00, 0x00, 0x30,
    0x10, 0x00, 0x00, 0x10, 0x30, 0x00, 0x00, 0x18, 0x60, 0x8e, 0xe3, 0x0c,
    0xc0, 0x8f, 0xe3, 0x07, 0x00, 0x8f, 0xe3, 0x01, 0x00, 0x1e, 0xf0, 0x00,
    0x00, 0xf0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define shoot_x 64
#define shoot_y 32
#define shoot_w 32
#define shoot_h 32

#define shutter_no_width 32
#define shutter_no_height 32
static const unsigned char shutter_no_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x0c, 0x00, 0x00, 0x38, 0x0e, 0x00, 0x00, 0x70, 0x07, 0x00,
    0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
    0x00, 0x70, 0x07, 0x00, 0x00, 0x38, 0x0e, 0x00, 0x00, 0x18, 0x0c, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

#define shutter_rdy_width 32
#define shutter_rdy_height 32
static const unsigned char shutter_rdy_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x38, 0x0e, 0x00,
    0x00, 0x0c, 0x18, 0x00, 0x00, 0x04, 0x10, 0x00, 0x00, 0x06, 0x30, 0x00,
    0x00, 0x02, 0x20, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x02, 0x20, 0x00,
    0x00, 0x06, 0x30, 0x00, 0x00, 0x04, 0x10, 0x00, 0x00, 0x0c, 0x18, 0x00,
    0x00, 0x38, 0x0e, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

#define shutter_act_width 32
#define shutter_act_height 32
static const unsigned char shutter_act_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf8, 0x0f, 0x00,
    0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfe, 0x3f, 0x00,
    0x00, 0xfe, 0x3f, 0x00, 0x00, 0xfe, 0x3f, 0x00, 0x00, 0xfe, 0x3f, 0x00,
    0x00, 0xfe, 0x3f, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfc, 0x1f, 0x00,
    0x00, 0xf8, 0x0f, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

#define shutter_trig_width 32
#define shutter_trig_height 32
static const unsigned char shutter_trig_bits[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xfc, 0x1f, 0x00,
    0x00, 0xfe, 0x3f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x80, 0xff, 0xff, 0x00,
    0x80, 0xff, 0xff, 0x00, 0xc0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x01,
    0xc0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x01,
    0xc0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x01, 0x80, 0xff, 0xff, 0x00,
    0x80, 0xff, 0xff, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xfe, 0x3f, 0x00,
    0x00, 0xfc, 0x1f, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

void showBitmap(unsigned char bitmapId, unsigned char color)
{
    switch (bitmapId) {
	case BM_NONE:
	    // No operation
	    break;
	case BM_SHOOT_NO:
	    display.drawXBitmap(shoot_x, shoot_y, shutter_no_bits, shutter_no_width, shutter_no_height, color);
	    break;
	case BM_SHOOT_RDY:
	    display.drawXBitmap(shoot_x, shoot_y, shutter_rdy_bits, shutter_rdy_width, shutter_rdy_height, color);
	    break;
	case BM_SHOOT_ACT:
	    display.drawXBitmap(shoot_x, shoot_y, shutter_act_bits, shutter_act_width, shutter_act_height, color);
	    break;
	case BM_SHOOT_TRIG:
	    display.drawXBitmap(shoot_x, shoot_y, shutter_trig_bits, shutter_trig_width, shutter_trig_height, color);
	    break;
	case BM_MODE_SINGLE:
	    display.drawXBitmap(bm_single_x, bm_single_y, bm_single_bits, bm_single_width, bm_single_height, color);
	    break;
	case BM_MODE_CLUSTER:
	    display.drawXBitmap(bm_cluster_x, bm_cluster_y, bm_cluster_bits, bm_cluster_width, bm_cluster_height, color);
	    break;
	case BM_MODE_VPAN:
	    display.drawXBitmap(bm_vpan_x, bm_vpan_y, bm_vpan_bits, bm_vpan_width, bm_vpan_height, color);
	    break;
	case BM_MODE_HPAN:
	    display.drawXBitmap(bm_hpan_x, bm_hpan_y, bm_hpan_bits, bm_hpan_width, bm_hpan_height, color);
	    break;
	case BM_MODE_QUAD:
	    display.drawXBitmap(bm_quad_x, bm_quad_y, bm_quad_bits, bm_quad_width, bm_quad_height, color);
	    break;
	case BM_MODE_360:
	    display.drawXBitmap(bm_360_x, bm_360_y, bm_360_bits, bm_360_width, bm_360_height, color);
	    break;
	case BM_CIRCLE:
	    display.drawXBitmap(circ_x, circ_y, circ_bits, circ_width, circ_height, color);
	    break;
	case BM_ARC_0:
	    display.drawXBitmap(arc_0_x, arc_0_y, arc_0_bits, arc_0_width, arc_0_height, color);
	    break;
	case BM_ARC_1:
	    display.drawXBitmap(arc_1_x, arc_1_y, arc_1_bits, arc_1_width, arc_1_height, color);
	    break;
	case BM_ARC_2:
	    display.drawXBitmap(arc_2_x, arc_2_y, arc_2_bits, arc_2_width, arc_2_height, color);
	    break;
	case BM_ARC_15:
	    display.drawXBitmap(arc_15_x, arc_15_y, arc_15_bits, arc_15_width, arc_15_height, color);
	    break;
	case BM_ARC_16:
	    display.drawXBitmap(arc_16_x, arc_16_y, arc_16_bits, arc_16_width, arc_16_height, color);
	    break;
	case BM_ARC_17:
	    display.drawXBitmap(arc_17_x, arc_17_y, arc_17_bits, arc_17_width, arc_17_height, color);
	    break;
	case BM_ARC_18:
	    display.drawXBitmap(arc_18_x, arc_18_y, arc_18_bits, arc_18_width, arc_18_height, color);
	    break;
	case BM_ARC_19:
	    display.drawXBitmap(arc_19_x, arc_19_y, arc_19_bits, arc_19_width, arc_19_height, color);
	    break;
	case BM_ARC_20:
	    display.drawXBitmap(arc_20_x, arc_20_y, arc_20_bits, arc_20_width, arc_20_height, color);
	    break;
	case BM_ARC_21:
	    display.drawXBitmap(arc_21_x, arc_21_y, arc_21_bits, arc_21_width, arc_21_height, color);
	    break;
	case BM_ARC_22:
	    display.drawXBitmap(arc_22_x, arc_22_y, arc_22_bits, arc_22_width, arc_22_height, color);
	    break;
	case BM_ARC_23:
	    display.drawXBitmap(arc_23_x, arc_23_y, arc_23_bits, arc_23_width, arc_23_height, color);
	    break;
    }
}

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

#define REFRESH_INTERVAL 25

struct LcdState_s {
    unsigned char flags;
    signed char pan;  // 0-23
    signed char tilt; // -3 - 8
    unsigned char shots;
    unsigned char shootMode;
    unsigned char shutterState;
    unsigned char shutterStateLcd;
    unsigned char sinceRefresh;
    boolean vertical;
    boolean autokap;
    boolean priorityRefresh;
};
struct LcdState_s lcdState = {
    // flags
    REFRESH_PAN_TILT | REFRESH_AUTO_COUNT | REFRESH_SHOOT_MODE | 
    REFRESH_SHUTTER | REFRESH_HOVER,
    0,    // pan
    0,    // tilt
    0,    // shots
    0,    // shootMode
    0,    // shutterState
    0,    // sinceRefresh
    false,  // vertical
    false,  // autokap
    false,  // priority refresh
};

#define MAXSHOTS 6

enum ShutterState_e {
    SHUTTER_STATE_NO,
    SHUTTER_STATE_RDY,
    SHUTTER_STATE_ACT,
    SHUTTER_STATE_TRIG,

    // keep this last
    NUM_SHUTTER_STATES,
};

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


void showShutter()
{
    unsigned char bitmapId;

    // Serial.println(m);
    switch (lcdState.shutterState) {
	case SHUTTER_STATE_NO:
	    bitmapId = BM_SHOOT_NO;
	    break;
	case SHUTTER_STATE_RDY:
	    bitmapId = BM_SHOOT_RDY;
	    break;
	case SHUTTER_STATE_ACT:
	    bitmapId = BM_SHOOT_ACT;
	    break;
	case SHUTTER_STATE_TRIG:
	    bitmapId = BM_SHOOT_TRIG;
	    break;
	default:
	    bitmapId = BM_NONE;
	    break;
    }

    // erase old bitmap
    display.fillRect(shoot_x, shoot_y, 32, 32, WHITE);

    // show new bitmap
    showBitmap(bitmapId, BLACK);
}

void showShootMode()
{
    unsigned char bitmapId;

    // Serial.println(m);
    switch (lcdState.shootMode) {
	case MODE_SINGLE:
	    bitmapId = BM_MODE_SINGLE;
	    break;
	case MODE_CLUSTER:
	    bitmapId = BM_MODE_CLUSTER;
	    break;
	case MODE_VPAN:
	    bitmapId = BM_MODE_VPAN;
	    break;
	case MODE_HPAN:
	    bitmapId = BM_MODE_HPAN;
	    break;
	case MODE_QUAD:
	    bitmapId = BM_MODE_QUAD;
	    break;
	case MODE_360:
	    bitmapId = BM_MODE_360;
	    break;
	default:
	    bitmapId = BM_NONE;
	    break;
    }

    int color = WHITE;  // normal background color
    if (lcdState.flags & INV_SHOOT_MODE) {
	color = !color;  // invert color scheme
    }

    // erase old bitmap
    display.fillRect(MODE_OFFSET_X, MODE_OFFSET_Y, 32, 32, color);

    // show new bitmap
    color = !color;
    showBitmap(bitmapId, color);
}

#define SHOTS_ORIGIN_X 0
#define SHOTS_ORIGIN_Y 0
#define SHOTS_WIDTH 64
#define SHOTS_HEIGHT 32
#define SHOTS_CURSOR_X 8
#define SHOTS_CURSOR_Y 8

void showShots()
{
    char buf[6];

    // create text in buffer
    if (lcdState.autokap) {
	strcpy_P(buf, (const char *)F("AUTO"));
    }
    else if (lcdState.flags & INV_AUTO_COUNT) {
	// This control is highlighted, so show "MAN" instead of count
	strcpy_P(buf, (const char *)F(" MAN"));
    }
    else {
	sprintf_P(buf, (const char *)F("%4d"), lcdState.shots);
    }

    // erase earlier content, set text color based on inverse state
    if (lcdState.flags & INV_AUTO_COUNT) {
	display.fillRect(SHOTS_ORIGIN_X, SHOTS_ORIGIN_Y, SHOTS_WIDTH-1, SHOTS_HEIGHT-1, BLACK);
	display.setTextColor(WHITE, BLACK);
    }
    else {
	display.fillRect(SHOTS_ORIGIN_X, SHOTS_ORIGIN_Y, SHOTS_WIDTH-1, SHOTS_HEIGHT-1, WHITE);
	display.setTextColor(BLACK, WHITE);
    }

    // position cursor
    display.setCursor(SHOTS_CURSOR_X+SHOTS_ORIGIN_X, SHOTS_CURSOR_Y+SHOTS_ORIGIN_Y);

    // display it!
    display.print(buf); 
}

#define HOVER_ORIGIN_X 64
#define HOVER_ORIGIN_Y 64
#define HOVER_HOR_X 5
#define HOVER_HOR_Y 8
#define HOVER_HOR_W 22
#define HOVER_HOR_H 14
#define HOVER_VER_X (HOVER_HOR_Y)
#define HOVER_VER_Y (HOVER_HOR_X)
#define HOVER_VER_W (HOVER_HOR_H)
#define HOVER_VER_H (HOVER_HOR_W)

void showHoVer()
{
    unsigned char color;

    // clear the Ho/Ver area
    color = WHITE;  // assume we erase with white
    if (lcdState.flags & INV_HOVER) {
	color = !color;  // erase with black
    }
    display.fillRect(HOVER_ORIGIN_X, HOVER_ORIGIN_Y, 31, 31, color);

    color = !color;  // draw with opposite as background
    if (lcdState.vertical) {
	// vertical
	display.drawRect(HOVER_VER_X+HOVER_ORIGIN_X, HOVER_VER_Y + HOVER_ORIGIN_Y, HOVER_VER_W, HOVER_VER_H, color);
	display.drawRect(HOVER_VER_X+HOVER_ORIGIN_X+1, HOVER_VER_Y+HOVER_ORIGIN_Y+1, HOVER_VER_W-2, HOVER_VER_H-2, color);
    }
    else {
	// horizontal
	display.drawRect(HOVER_HOR_X+HOVER_ORIGIN_X, HOVER_HOR_Y + HOVER_ORIGIN_Y, HOVER_HOR_W, HOVER_HOR_H, color);
	display.drawRect(HOVER_HOR_X+HOVER_ORIGIN_X+1, HOVER_HOR_Y+HOVER_ORIGIN_Y+1, HOVER_HOR_W-2, HOVER_HOR_H-2, color);
    }
}

// Triangle vertices for each view of the pan indicator.
struct PanInd_s {
    uint8_t x0;
    uint8_t y0;
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
};

const struct PanInd_s panIndicator[24] PROGMEM =
{
    {56,64,16,72,16,57}, // 0
    {55,58,19,75,15,61},
    {52,53,23,78,15,65},
    {48,48,27,80,16,69},
    {43,44,31,81,18,73},
    {38,41,35,81,21,77},
    {32,40,40,80,24,80}, // 6
    {26,41,43,77,29,81},
    {20,44,46,73,33,81},
    {16,48,48,69,37,80},
    {12,52,49,65,41,78},
    {9,58,49,61,45,75},
    {8,64,48,56,48,72}, // 12
    {9,70,45,53,49,67},
    {12,76,41,50,49,63},
    {16,80,37,48,48,59},
    {20,84,33,47,46,55},
    {26,87,29,47,43,51},
    {32,88,24,49,39,48}, // 18
    {38,87,21,51,35,47},
    {43,84,18,55,31,47},
    {48,80,16,59,27,48},
    {52,76,15,63,23,50},
    {55,70,15,67,19,53},
};

void showPan(int pan) {
    static uint16_t x0 = 0;
    static uint16_t y0 = 0;
    static uint16_t x1 = 0;
    static uint16_t y1 = 0;
    static uint16_t x2 = 0;
    static uint16_t y2 = 0;

    // erase old triangle
    display.fillTriangle(x0, y0, x1, y1, x2, y2, WHITE);

    // clear pan area
    // Serial.println(pan);
    x0 = pgm_read_byte(&panIndicator[pan].x0);
    y0 = pgm_read_byte(&panIndicator[pan].y0);
    x1 = pgm_read_byte(&panIndicator[pan].x1);
    y1 = pgm_read_byte(&panIndicator[pan].y1);
    x2 = pgm_read_byte(&panIndicator[pan].x2);
    y2 = pgm_read_byte(&panIndicator[pan].y2);

    // draw new triangle
    display.fillTriangle(x0, y0, x1, y1, x2, y2, BLACK);
}

void showTilt(unsigned char tilt) 
{
    static unsigned char old_bitmapId = BM_NONE;
    unsigned char bitmapId;

    switch (tilt) {
	case 0:
	    bitmapId = BM_ARC_0;
	    break;
	case 1:
	    bitmapId = BM_ARC_1;
	    break;
	case 2:
	    bitmapId = BM_ARC_2;
	    break;
	case 15:
	    bitmapId = BM_ARC_15;
	    break;
	case 16:
	    bitmapId = BM_ARC_16;
	    break;
	case 17:
	    bitmapId = BM_ARC_17;
	    break;
	case 18:
	    bitmapId = BM_ARC_18;
	    break;
	case 19:
	    bitmapId = BM_ARC_19;
	    break;
	case 20:
	    bitmapId = BM_ARC_20;
	    break;
	case 21:
	    bitmapId = BM_ARC_21;
	    break;
	case 22:
	    bitmapId = BM_ARC_22;
	    break;
	case 23:
	    bitmapId = BM_ARC_23;
	    break;
	default:
	    bitmapId = BM_NONE;
    }

    // erase old bitmap
    showBitmap(old_bitmapId, WHITE);

    // show new bitmap
    showBitmap(bitmapId, BLACK);

    // remember old for next time
    old_bitmapId = bitmapId;
}

void writeLcd() 
{
    bool update = false;
    bool refresh = false;

    if (lcdState.priorityRefresh) {
	// doing refresh is a priority
	refresh = true;
    }
    else if (lcdState.flags & (CHANGE_FLAGS)) {
	// refresh is not a priority but an update is needed
	update = true;
    }
    else if (lcdState.sinceRefresh >= REFRESH_INTERVAL) {
	// do periodic refresh now just because it's time.
	refresh = true;
    }

    if (update) {
	// Serial.println("Update.");
	if (lcdState.flags & REFRESH_PAN_TILT) {
	    showPan(lcdState.pan);
	    showTilt(lcdState.tilt);
	    lcdState.flags &= ~REFRESH_PAN_TILT;
	}
	if (lcdState.flags & REFRESH_HOVER) {
	    showHoVer();
	    lcdState.flags &= ~REFRESH_HOVER;
	}
	if (lcdState.flags & REFRESH_SHUTTER) {
	    showShutter();
	    lcdState.flags &= ~REFRESH_SHUTTER;
	}
	if (lcdState.flags & REFRESH_SHOOT_MODE) {
	    showShootMode();
	    lcdState.flags &= ~REFRESH_SHOOT_MODE;
	}
	if (lcdState.flags & REFRESH_AUTO_COUNT) {
	    showShots();
	    lcdState.flags &= ~REFRESH_AUTO_COUNT;
	}
	lcdState.priorityRefresh = true;
    }

    if (refresh) {
	// Serial.println("Refresh.");
	display.refresh();
	lcdState.sinceRefresh = 0;
	lcdState.priorityRefresh = false;
    } else {
	lcdState.sinceRefresh++;
    }
}

#define ARRAY_LEN(a) ((sizeof(a)) / (sizeof(a[0])))

#define ACCEL_PAN (1)
#define ACCEL_TILT (1)

#define PPM_OUT (10)
#define TX_PAIR (15)

// Values for Timer1 Config registers
#define WGM_15_1A (0x03)
#define WGM_15_1B (0x03 << 3)
#define COM1A_00 (0x00 << 6)
#define COM1B_10 (0x02 << 4)
#define COM1B_11 (0x03 << 4)
#define COM1C_00 (0x00 << 2)
#define CS1_DIV8 (0x02)
#define TIMSK1_TOIE (0x01)

// ------------------------------------------------------------------------------------------
// PPM Generation

#define PPM_CHANNELS (6)
#define PPM_PHASES (PPM_CHANNELS+1)
#define PPM_RESYNC_PHASE (0)
#define PPM_PULSE_WIDTH (800)  // 400uS
#define PPM_CENTER (3000)      // 1.5mS
// #define PPM_FRAME_LEN (64000)  // 32mS -> ~30Hz
#define PPM_FRAME_LEN (40000)  // 20ms -> 50Hz

#define PPM_CHAN_PAN (1)
#define PPM_CHAN_TILT (2)
#define PPM_CHAN_SHUTTER (3)
#define PPM_CHAN_HOVER (4)
#define PPM_CHAN_UNUSED2 (5)
#define PPM_CHAN_UNUSED3 (6)

struct Ppm_s {
    int phase; // which phase of PPM we are in, 0-6.  6 is long resync phase. 
    int time[7 /* PPM_PHASES */];  // width of each PPM phase (1 = 0.5uS)
    bool startCycle;
} ppm;

void ppmSetup()
{  
    pinMode(PPM_OUT, OUTPUT);
    pinMode(TX_PAIR, INPUT);

    // Init PPM phases
    int remainder = PPM_FRAME_LEN;
    for (ppm.phase = 1; ppm.phase <= PPM_CHANNELS; ppm.phase++) {
	ppm.time[ppm.phase] = PPM_CENTER;  // 1.5mS
	remainder -= ppm.time[ppm.phase];
    }
    ppm.time[PPM_RESYNC_PHASE] = remainder;
    ppm.phase = 0;
    ppm.startCycle = true;

    // Disable Interrupts
    cli();

    // Init for PPM Generation

    TCCR1A = WGM_15_1A | COM1A_00 | COM1B_11 | COM1C_00; // Fast PWM with OCR1A defining TOP
    TCCR1B = WGM_15_1B | CS1_DIV8;                       // Prescaler : system clock / 8.
    TCCR1C = 0;                                          // Not used
    OCR1A = ppm.time[ppm.phase];                           // Length of current phase
    OCR1B = PPM_PULSE_WIDTH;                             // Width of pulses
    // OCR1C = 0;                                           // Not used
    TIMSK1 = TIMSK1_TOIE;                                // interrupt on overflow (end of phase)

    // Enable interrupts
    sei();
}

void ppmWrite(int periods[7 /*PPM_PHASES*/])
{
    for (int i = 1; i <= PPM_CHANNELS; i++) {
	ppm.time[i] = periods[i];
    }
}

ISR(TIMER1_OVF_vect) 
{
    static unsigned remainder = PPM_FRAME_LEN;

    // Handle Timer 1 overflow: Start of PPM interval
    // Program total length of this phase in 0.5uS units.
    ppm.phase += 1;
    if (ppm.phase > PPM_CHANNELS) {
	ppm.phase = 0;
	ppm.startCycle = true;
    }

    // Remainder computation
    if (ppm.phase == 0) {
	// Store remainder
	ppm.time[ppm.phase] = remainder;
	remainder = PPM_FRAME_LEN;
    }
    else {
	remainder -= ppm.time[ppm.phase];
    }

    // Program next interval time.
    OCR1A = ppm.time[ppm.phase];
}

// ------------------------------------------------------------------------------------------
// Utility functions

int iabs(int x)
{
    if (x < 0) {
	return -x;
    }
    return x;
}

// --------------------------------------------------------------------------------
// Joystick

#define JS_BUTTON (9)
#define JS_X (A0)
#define JS_Y (A1)
#define JS_DEBOUNCE (5)
#define JS_MID (512)
#define JS_MAX (1023)

#define JS_NEUTRAL (350) // (450)
#define JS_PLUS (400) // (500)
#define JS_SLIDE_THRESH (100)

struct Js_s {
    int x;
    int y;
    int slideStart_x;
    int slideStart_y;
    bool but;
    bool pressed;
    unsigned int debounce;
} js;

void jsSetup()
{
    pinMode(JS_BUTTON, INPUT_PULLUP);
    js.but = false;
    js.pressed = false;
    js.debounce = 0;
}

void jsPoll()
{
    bool oldbut;
    js.x = JS_MID - analogRead(JS_X);
    js.y = JS_MID - analogRead(JS_Y);
    oldbut = js.but;
    js.but = digitalRead(JS_BUTTON);
    if (js.debounce > 0) js.debounce--;
    if (!js.but & oldbut & (js.debounce == 0)) {
	js.pressed = true;
	js.debounce = JS_DEBOUNCE;
    }
}

bool jsIsOut()
{
    long r2 = ((long)js.x*js.x) + ((long)js.y*js.y);
    return r2 > (long)JS_PLUS*JS_PLUS;
}

bool jsIsCenter()
{
    long r2 = ((long)js.x*js.x) + ((long)js.y*js.y);
    return (r2 < (long)JS_NEUTRAL*JS_NEUTRAL);
    // return ((iabs(js.x) < JS_NEUTRAL) &&
    //         (iabs(js.y) < JS_NEUTRAL));
}

void jsSetSlideStart()
{
    js.slideStart_x = js.x;
    js.slideStart_y = js.y;
}

bool jsIsSlidingLR()
{
    int move = iabs(js.x - js.slideStart_x);
    return (move > JS_SLIDE_THRESH);
}

bool jsIsSlidingUD()
{
    int move = iabs(js.y - js.slideStart_y);
    return (move > JS_SLIDE_THRESH);
}

#define TAN_37_5 (0.76732699)
#define TAN_33_75 (0.66817864)
#define TAN_22_5 (0.41421356)
#define TAN_11_25 (0.19891237)
#define TAN_7_5 (0.13165250)

// map joystick x, y into sectors 0-23, in standard orientation
// (0 is due "east", 6 is north, 12 west, 18 south.)
int jsGetIndex24()
{
    int index = 0;
    int x = js.x;
    int y = js.y;
    bool fold_y = false;
    bool fold_x = false;
    bool fold_xy = false;
    int tmp;

    // fold x, y into 0, 1 positions
    if (y < 0) {
	fold_x = true;
	y = -y;
    }
    if (x < 0) {
	fold_y = true;
	x = -x;
    }
    if (y > x) {
	fold_xy = true;
	tmp = x;
	x = y;
	y = tmp;
    }

    float t = (float)y/(float)x;
    if (t > TAN_22_5) {
	if (t > TAN_37_5) {
	    index = 3;
	}
	else {
	    index = 2;
	}
    }
    else {
	if (t > TAN_7_5) {
	    index = 1;
	}
	else {
	    index = 0;
	}
    }

    // unfold as necessary
    if (fold_xy) index = 6 - index;
    if (fold_y) index = 12 - index;
    if (fold_x) index = 24 - index;
    if (index == 24) index = 0;

    return index;
}

// map joystick x, y into sectors 0-15, in standard orientation
// (0 is due "east", 4 is north, 8 west, 12 south.)
int jsGetIndex16()
{
    int index = 0;
    int x = js.x;
    int y = js.y;
    bool fold_y = false;
    bool fold_x = false;
    bool fold_xy = false;
    int tmp;

    // fold x, y into 0, 1 positions
    if (y < 0) {
	fold_x = true;
	y = -y;
    }
    if (x < 0) {
	fold_y = true;
	x = -x;
    }
    if (y > x) {
	fold_xy = true;
	tmp = x;
	x = y;
	y = tmp;
    }

    float t = (float)y/(float)x;
    if (t > TAN_22_5) {
	index = 1;
    }

    // unfold as necessary
    if (fold_xy) index = 3 - index;
    if (fold_y) index = 7 - index;
    if (fold_x) index = 15 - index;

    return index;
}

// map joystick x, y into sectors 0-15, in standard orientation
// (0 is due "east", 4 is north, 8 west, 12 south.)
// difference between this and jsGetIndex16() is that these regions are centered on
// the cardinal directions.  
int jsGetIndex16_offs()
{
    int index = 0;
    int x = js.x;
    int y = js.y;
    bool fold_y = false;
    bool fold_x = false;
    bool fold_xy = false;
    int tmp;

    // fold x, y into 0, 1 positions
    if (y < 0) {
	fold_x = true;
	y = -y;
    }
    if (x < 0) {
	fold_y = true;
	x = -x;
    }
    if (y > x) {
	fold_xy = true;
	tmp = x;
	x = y;
	y = tmp;
    }

    float t = (float)y/(float)x;
    if (t < TAN_22_5) {    
      if (t < TAN_11_25) {
	index = 0;
      }
      else {
          index = 1;
      }
    }
    else {
      if (t < TAN_33_75) {
        index = 1;
      }
      else {
        index = 2;
      }
    }

    // unfold as necessary
    if (fold_xy) index = 4 - index;
    if (fold_y) index = 8 - index;
    if (fold_x) index = 16 - index;
    if (index == 16) index = 0;

    return index;
}

// -------------------------------------------------------------
// KAP Tx Model

#define ANG_360 (24)
#define TILT_MIN (15)
#define TILT_MAX (2)
#define TILT_MID_DEAD (8)
#define PAN_MIN (0)
#define PAN_MAX (23)

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

// #define SHOOT_IDLE (0)
// #define SHOOT_ACTIVE (1)

#define NEUTRAL_DISP_TIME (25)
#define DISP_NONE (0)
#define DISP_PAN (1)
#define DISP_TILT (2)
#define DISP_MODE (3)
#define DISP_HOVER (4)

#define SLEW_STABLE (0)
#define SLEW_MOVING (1)
#define SLEW_STABILIZING (2)

#define TICKS_PER_SECOND (50)
#define SECONDS(n) (n * TICKS_PER_SECOND)
#define TIME_STABILIZING (10)
#define TIME_SHUTTER_DOWN (5)
#define TIME_SHUTTER_POST (35)
#define TIME_JS_INITIAL (25)
#define TIME_JS_REPEAT (3)

#define SHUTTER_IDLE (0)
#define SHUTTER_TRIGGERED (1)
#define SHUTTER_DOWN (2)
#define SHUTTER_POST (3)

#define SHUTTER_DOWN_POS (3600)  // 1.8ms
#define SHUTTER_UP_POS (2400)    // 1.2ms
#define HOVER_HOR_POS (3600)     // 1.8ms
#define HOVER_VERT_POS (2400)    // 1.2ms

#define SHOT_QUEUE_LEN (36)

struct PanTilt_s {
    int pan;
    int tilt;
};

struct KapTx_s {
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

    // states
    unsigned char jsState;
    unsigned char slewState;
    unsigned char shutterState;

    // timers
    unsigned int jsTimer;
    unsigned int slewTimer;
    unsigned int shutterTimer;

    unsigned int shootMode;
    unsigned int shootMode_disp;

    unsigned int shotsQueued;
    struct PanTilt_s shotQueue[SHOT_QUEUE_LEN];

    boolean vertical;
    boolean autokap;
} model;

void modelSetup()
{
    model.userPos.pan = 6;  // facing away from operator
    model.userPos.tilt = 0;  // facing horizontal.

    model.shutterServo = SHUTTER_UP_POS;

    model.shootMode = MODE_SINGLE;
    model.shotsQueued = 0;

    model.vertical = false;
    model.autokap = false;
}

int addPan(int pan, int n)
{
  pan += n;
  if (pan < 0) pan += ANG_360;
  if (pan >= ANG_360) pan -= ANG_360;
  
  return pan;
}

int addTilt(int tilt, int n)
{
  tilt += n;
  if (tilt >= ANG_360) tilt -= ANG_360;
  if (tilt < 0) tilt += ANG_360;
  
  if ((tilt > TILT_MAX) && (tilt < TILT_MID_DEAD)) tilt = TILT_MAX;
  if ((tilt < TILT_MIN) && (tilt >= TILT_MID_DEAD)) tilt = TILT_MIN;
  
  return tilt;
}

void queueShotPwm(struct PanTilt_s *aimPoint)
{
    // bail out if queue is full
    if (model.shotsQueued == SHOT_QUEUE_LEN) return;

    struct PanTilt_s *entry = model.shotQueue + model.shotsQueued;
    *entry = *aimPoint;
    model.shotsQueued++;
}

void queueShot(struct PanTilt_s *aimPoint)
{
    struct PanTilt_s aimPointPwm;
    toPwm(&aimPointPwm, aimPoint);
    
    queueShotPwm(&aimPointPwm);
}

void dequeueShot()
{
    for (int n = 0; n < SHOT_QUEUE_LEN-2; n++) {
	model.shotQueue[n] = model.shotQueue[n+1];
    }
    model.shotsQueued--;
}

void adjPan(int adj)
{
    model.userPos.pan += adj;
    if (model.userPos.pan > PAN_MAX) {
	model.userPos.pan -= (PAN_MAX+1);
    }
    if (model.userPos.pan < PAN_MIN) {
	model.userPos.pan += (PAN_MAX+1);
    }

    // Serial.print("pan = ");
    // Serial.println(model.userPos.pan);
}

// Note: this is intended to work with adjustments of +/- 1.
void adjTilt(int adj)
{
    if ((adj > 0) && (model.userPos.tilt == TILT_MAX)) return;  // no change
    if ((adj < 0) && (model.userPos.tilt == TILT_MIN)) return;  // no change
    model.userPos.tilt += adj;
    if (model.userPos.tilt >= 24) model.userPos.tilt -= 24;
    if (model.userPos.tilt < 0) model.userPos.tilt += 24;
}

void setJsPan()
{
    int index = jsGetIndex24();

    model.userPos.pan = index;
}

void setJsTilt()
{
    int index = jsGetIndex24();
    
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

void setJsMode()
{
    int mode;
    
    switch (jsGetIndex24()) {
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
        mode = model.shootMode;
        break;
      default:
        // otherwise, don't change shootMode_disp
        mode = model.shootMode_disp;
    }
    
    // set display mode
    model.shootMode_disp = mode;
}

void setJsHoVer()
{
    int i = jsGetIndex16();

    if ((i == 0) || (i == 7) || (i == 8) || (i == 15)) {
	// set in horizontal mode
	model.vertical = false;
    }
    if ((i == 3) || (i == 4) || (i == 11) || (i == 12)) {
	// set in vertical mode
	model.vertical = true;
    }
}

void setJsAuto()
{
    int i = jsGetIndex16();

    if ((i == 3) || (i == 4)) {
	// set in auto mode
	model.autokap = true;
    }
    if ((i == 7) || (i == 8)) {
	// turn off auto mode
	model.autokap = false;
    }
}

// Interpret joystick motion as operator intentions
void jsUpdate()
{
    switch(model.jsState) {
	case JS_IDLE:
	    if (jsIsOut()) {
		// int pos = jsGetIndex24();
                int pos = jsGetIndex16_offs();
                
                switch (pos) {
		    case 0:
			// joystick is right
			model.jsState = JS_RIGHT;
			jsSetSlideStart();
			break;
		    case 2:
			// joystick is NE, mode select
			model.jsState = JS_SET_MODE;
			jsSetSlideStart();
			break;
		    case 4:
			// joystick is N, tilt 
			model.jsState = JS_UP;
			jsSetSlideStart();
			break;
		    case 6:
                        // joystick is NW, set Auto
                        model.jsState = JS_SET_AUTO;
			break;
		    case 8:
			// joystick is left
			model.jsState = JS_LEFT;
			jsSetSlideStart();
			break;
                    case 10:
                        // future home of config menu
                        break;
		    case 12:
			// joystick is down
			model.jsState = JS_DOWN;
			jsSetSlideStart();
			break;
                    case 14:
                    	// joystick is SE, HoVer select
			model.jsState = JS_SET_HOVER;
                        break;
		    default:
			// joystick is somewhere we don't care about
			break;
		}
	    }
	    break;
	case JS_RIGHT:
	    if (jsIsCenter()) {
		// was a bump right
		adjPan(-1);
		model.jsState = JS_IDLE;
	    }
	    if (jsIsSlidingUD()) {
		model.jsState = JS_SET_PAN;
		setJsPan();
	    }
	    break;
	case JS_LEFT:
	    if (jsIsCenter()) {
		// was a bump left
		adjPan(1);
		model.jsState = JS_IDLE;
	    }
	    if (jsIsSlidingUD()) {
		model.jsState = JS_SET_PAN;
		setJsPan();
	    }
	    break;
	case JS_UP:
	    if (jsIsCenter()) {
		// was a bump up
		adjTilt(1);
		model.jsState = JS_IDLE;
	    }
	    if (jsIsSlidingLR()) {
		model.jsState = JS_SET_TILT;
		setJsTilt();
	    }
	    break;
	case JS_DOWN:
	    if (jsIsCenter()) {
		// was a bump down
		adjTilt(-1);
		model.jsState = JS_IDLE;
	    }
	    if (jsIsSlidingLR()) {
		model.jsState = JS_SET_TILT;
		setJsTilt();
	    }
	    break;
	case JS_SET_PAN:
	    if (jsIsCenter()) {
		// JS Pan is done
		model.jsState = JS_IDLE;
	    }
	    else {
		// Continue tracking joystick pan
		setJsPan();
	    }
	    break;
	case JS_SET_TILT:
	    if (jsIsCenter()) {
		// JS Tilt is done
		model.jsState = JS_IDLE;
	    }
	    else {
		// Continue tracking joystick tilt
		setJsTilt();
	    }
	    break;
	case JS_SET_MODE:
	    if (jsIsCenter()) {
		// set new mode
                model.shootMode = model.shootMode_disp;
		// JS Mode is done
		model.jsState = JS_IDLE;
	    }
	    else {
		// Continue tracking joystick mode
		setJsMode();
	    }
	    break;
	case JS_SET_HOVER:
	    if (jsIsCenter()) {
		model.jsState = JS_IDLE;
	    }
	    else {
		// Track HOVER setting
		setJsHoVer();
	    }
	    break;
        case JS_SET_AUTO:
            if (jsIsCenter()) {
                model.jsState = JS_IDLE;
            }
            else {
                // Track AUTO setting
                setJsAuto();
            }
            break;
	default:
	    // bad state -- fix it
	    model.jsState = JS_IDLE;
	    break;
    }
}

void shootSingle()
{
  // queue a shot based on current pan/tilt
  queueShot(&model.userPos);
}

boolean isValidPanTilt(struct PanTilt_s *pt)
{
    // comparison looks wrong but it's actually right.
    // TILT_MAX = 2, TILT_MIN = 15.  Unreachable zone is 3-14.
    if ((pt->tilt > TILT_MAX) && (pt->tilt < TILT_MIN)) {
	return false;
    }

    return true;
}

#define SEQ_LEN 7

void shootCluster()
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

            sprintf(s, "queue shot %d, %d.", aimPoint.pan, aimPoint.tilt);
            Serial.println(s);
            queueShot(&aimPoint);
	}
}

void shootVpan()
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

void shootHpan()
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

void shootQuad()
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

void shoot360()
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

void shootUpdate()
{
  bool trigger = false;
  
  if (model.autokap) {
    // trigger new sequence when prior one finishes
    if (model.shotsQueued == 0) {
      trigger = true;
    }
  }
  else {
    // trigger new sequence when operator presses joystick
    if (js.pressed) {
      // reset button press flag
      js.pressed = false;
      trigger = true;
    }
  }

  if (trigger) {
    // perform shot processing based on mode
    switch (model.shootMode) {
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
            model.shootMode = MODE_SINGLE;
	    break;
    }
  }
}

#if 0   // obsolete after introduction of factor/offset params
typedef struct {
    int user;
    int pwm;
} UserToPwmEntry;

const UserToPwmEntry panTable[] = {  // -1600 ok, 
    {18, 1600},    // 18
    {17, 1466},
    {16, 1333},
    {15, 1200},
    {14, 1066},
    {13, 933},
    {12, 800},      // 12
    {11, 666},
    {10, 533},
    { 9, 400},
    { 8, 266},
    { 7, 133},
    { 6, 0},         // 6
    { 5, -133},
    { 4, -266},
    { 3, -400},
    { 2, -533},
    { 1, -666},
    { 0, -800},       // 0
    {23, -933},       // 23
    {22, -1066},
    {21, -1200},
    {20, -1333},
    {19, -1466},
    {18, -1600},      // 18
};

        // TODO-DW : recover RAM!
UserToPwmEntry tiltTable[] = {
    {15, -1496},  // -45
    {16, -1231},
    {17, -965},
    {18, -700},   // down
    {19, -434},
    {20, -168},
    {21, 97},   // +45
    {22, 362},
    {23, 628},
    { 0, 893},   // +90
    { 1, 1159},   // +105
    { 2, 1424},   // +120
};
#endif

#ifdef 0
void toPwm(struct PanTilt_s *pwm, const struct PanTilt_s *user)
{
    bool foundMatch;
    int panPwm = 0;
    int tiltPwm = 0;

    // find pan closest to current pan matching current user pos
    foundMatch = false;
    // Serial.print("userPos.pan =");
    // Serial.println(model.userPos.pan); 
    for (unsigned i = 0; i < ARRAY_LEN(panTable); i++) {
	if (panTable[i].user == user->pan) {
	    // Serial.print("panTable:");
	    // Serial.println(i);

	    // Found a match of user pos
	    if (!foundMatch) {
		// This is first match, adopt it
		panPwm = panTable[i].pwm;
                foundMatch = true;
	    }
	    else {
		// Compare against previous match
		int currDelta = panPwm - model.servoPos.pan;
		int newDelta = panTable[i].pwm - model.servoPos.pan;
		if (abs(newDelta) < abs(currDelta)) {
		    // switch to new one
		    panPwm = panTable[i].pwm;
		}
	    }
	}
    }

    // find tilt closest to current tilt matching current user pos
    foundMatch = false;
    for (unsigned i = 0; i < ARRAY_LEN(tiltTable); i++) {
	if (tiltTable[i].user == user->tilt) {
	    // Found a match of user pos
	    if (!foundMatch) {
		// This is first match, adopt it
		tiltPwm = tiltTable[i].pwm;
	    }
	    else {
		// Compare against previous match
		int currDelta = tiltPwm - model.servoPos.tilt;
		int newDelta = tiltTable[i].pwm - model.servoPos.tilt;
		if (abs(newDelta) < abs(currDelta)) {
		    // switch to new one
		    tiltPwm = tiltTable[i].pwm;
		}
	    }
	    foundMatch = true;
	}
    }

    // Set servoPos components
    pwm->pan = panPwm;
    pwm->tilt = tiltPwm;
    // Serial.print("goalPos.pan: ");
    // Serial.print(model.servoGoalPos.pan);
    // Serial.print(", goalPos.tilt: ");
    // Serial.print(model.servoGoalPos.tilt);
    // Serial.println();
}
#else


void toPwm(struct PanTilt_s *pwm, const struct PanTilt_s *user)
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
              currDelta = abs(pwm - model.servoPos.pan);
          } else {
              newDelta = abs(pwm - model.servoPos.pan);
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
#endif

boolean atGoalPos()
{
    struct PanTilt_s goal;
    getGoalPwm(&goal);

    return ((goal.pan == model.servoPos.pan) &&
	    (goal.tilt == model.servoPos.tilt) &&
	    (model.servoVel.pan == 0) &&
	    (model.servoVel.tilt == 0));
}

void slew(int *x, int *v, int target, int a)
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

void getGoalPwm(struct PanTilt_s *goal)
{
    if (model.shotsQueued) {
	// slew target is the head of shot queue
	*goal = model.shotQueue[0];
    }
    else {
	// slew target is user position
	toPwm(goal, &model.userPos);
    }
}

void moveServos()
{
    // update servo target position from userPos
    struct PanTilt_s goal;

    getGoalPwm(&goal);

    slew(&model.servoPos.pan, &model.servoVel.pan, goal.pan, ACCEL_PAN);
    slew(&model.servoPos.tilt, &model.servoVel.tilt, goal.tilt, ACCEL_TILT);
}

void slewUpdate()
{
    // execute slew state machine to bring servos to target position.
    switch (model.slewState) {
	case SLEW_STABLE:
	    // Serial.println("slew state STABLE");
	    if ((model.shutterState == SHUTTER_IDLE) &&
		(!atGoalPos())) {
		// start moving
		model.slewState = SLEW_MOVING;
		moveServos();
	    }
	    break;
	case SLEW_MOVING:
	    // Serial.println("slew state MOVING");
	    if (!atGoalPos()) {
		moveServos();
	    } else {
		model.slewTimer = TIME_STABILIZING;
		model.slewState = SLEW_STABILIZING;
	    }
	    break;
	case SLEW_STABILIZING:
	    // Serial.println("slew state STABILIZING");
	    if (!atGoalPos()) {
		model.slewTimer = 0;
		model.slewState = SLEW_MOVING;
		moveServos();
	    }
	    else if (model.slewTimer > 0) {
		// Wait for slew stabilization time
		model.slewTimer--;
	    } else {
		// waiting is done, we're stable
		model.slewState = SLEW_STABLE;
	    }
	    break;
	default:
	    // Serial.println("slew state default!");
	    // This should not have happened!
	    model.slewState = SLEW_STABLE;
	    break;
    }
}

void shutterUpdate()
{
    switch (model.shutterState) {
	case SHUTTER_IDLE:
	    // Serial.println("shutter state IDLE");
	    if (model.shotsQueued != 0) {
		// transition to TRIGGERED state
		model.shutterState = SHUTTER_TRIGGERED;
	    }
	    break;
	case SHUTTER_TRIGGERED:
	    // Serial.println("shutter state TRIGGERED");
	    if (model.slewState == SLEW_STABLE) {
		// trip shutter and transition to DOWN state
		model.shutterServo = SHUTTER_DOWN_POS;
		model.shutterTimer = TIME_SHUTTER_DOWN;
		model.shutterState = SHUTTER_DOWN;
		// Serial.println("shutter state DOWN");
	    }
	    break;
	case SHUTTER_DOWN:
	    // Serial.println("shutter state DOWN");
	    if (model.shutterTimer > 0) {
		model.shutterTimer--;
	    } else {
		// shutter has been down long enough
		model.shutterServo = SHUTTER_UP_POS;
		model.shutterTimer = TIME_SHUTTER_POST;
		model.shutterState = SHUTTER_POST;
		// Serial.println("shutter state POST");
	    }
	    break;
	case SHUTTER_POST:
	    // Serial.println("shutter state POST");
	    if (model.shutterTimer > 0) {
		model.shutterTimer--;
	    } else {
		// shutter has been up long enough
		dequeueShot();
		model.shutterState = SHUTTER_IDLE;
		// Serial.println("shutter state IDLE");
	    }
	    break;
	default:
	    // Serial.println("Bogus shutter state!");
	    model.shutterState = SHUTTER_IDLE;
	    break;
    }
}

void modelUpdate()
{
    // Do shot processing.
    shootUpdate();

    // update pan/tilt/shoot based on user input
    // panTiltUpdate();

    // Process joystick input into user intentions
    jsUpdate();

    // perform slew operations
    slewUpdate();

    // perform shooting operation
    shutterUpdate();
}

unsigned char getShutterLcdState()
{
  unsigned char state = 0;
  
  if (model.shutterState == SHUTTER_DOWN) {
    state = SHUTTER_STATE_TRIG;
  }
  else if ((model.shutterState == SHUTTER_POST) ||
           (model.shutterState == SHUTTER_TRIGGERED)) {
    state = SHUTTER_STATE_ACT;
  }
  else if (!model.autokap && 
           (model.slewState == SLEW_STABLE)) {
    state = SHUTTER_STATE_RDY;
  }
  else {
    state = SHUTTER_STATE_NO;
  }
  
  return state;
}

void modelWrite()
{
    // Set LCD state based on model, flagging changes when they happen.

    // Transfer pan and tilt changes
    unsigned char lcdPan = model.userPos.pan;
    if (lcdState.pan != model.userPos.pan) {
        lcdState.pan = model.userPos.pan;
        lcdState.flags |= REFRESH_PAN_TILT;
    }
    if (lcdState.tilt != model.userPos.tilt) {
        lcdState.tilt = model.userPos.tilt;
        lcdState.flags |= REFRESH_PAN_TILT;
    }

    // Transfer changes to HOVER
    if ((model.jsState == JS_SET_HOVER) && 
        ((lcdState.flags & INV_HOVER) == 0)) {
        lcdState.flags |= INV_HOVER;
        lcdState.flags |= REFRESH_HOVER;
    } 
    else if ((model.jsState != JS_SET_HOVER) && 
             ((lcdState.flags & INV_HOVER) != 0)) {
        lcdState.flags &= ~INV_HOVER;
        lcdState.flags |= REFRESH_HOVER;
    }
    if (model.vertical != lcdState.vertical) {
        lcdState.vertical = model.vertical;
        lcdState.flags |= REFRESH_HOVER;
    }

    // Transfer changes to shoot mode
    if ((model.jsState == JS_SET_MODE) && 
        ((lcdState.flags & INV_SHOOT_MODE) == 0)) {
        lcdState.flags |= INV_SHOOT_MODE;
        lcdState.flags |= REFRESH_SHOOT_MODE;
    } 
    else if ((model.jsState != JS_SET_MODE) && 
             ((lcdState.flags & INV_SHOOT_MODE) != 0)) {
        lcdState.flags &= ~INV_SHOOT_MODE;
        lcdState.flags |= REFRESH_SHOOT_MODE;
    }
    if (model.jsState == JS_SET_MODE) {
        // display shootMode_disp, not mode
        if (model.shootMode_disp != lcdState.shootMode) {
            lcdState.shootMode = model.shootMode_disp;
            lcdState.flags |= REFRESH_SHOOT_MODE;
        }
    }
    else {
        // display mode
        if (model.shootMode != lcdState.shootMode) {
            lcdState.shootMode = model.shootMode;
            lcdState.flags |= REFRESH_SHOOT_MODE;
        }
    }

    // Transfer changes to Autokap, count
    if ((model.jsState == JS_SET_AUTO) && 
        ((lcdState.flags & INV_AUTO_COUNT) == 0)) {
        lcdState.flags |= INV_AUTO_COUNT;
        lcdState.flags |= REFRESH_AUTO_COUNT;
    } 
    else if ((model.jsState != JS_SET_AUTO) && 
             ((lcdState.flags & INV_AUTO_COUNT) != 0)) {
        lcdState.flags &= ~INV_AUTO_COUNT;
        lcdState.flags |= REFRESH_AUTO_COUNT;
    }
    if ((lcdState.shots != model.shotsQueued) ||
        (lcdState.autokap != model.autokap)) {
        lcdState.autokap = model.autokap;
        lcdState.shots = model.shotsQueued;
        lcdState.flags |= REFRESH_AUTO_COUNT;
    }
        
    // Get displayed shutter state
    unsigned char state = getShutterLcdState();
    if (lcdState.shutterState != state) {
        lcdState.shutterState = state;
        lcdState.flags |= REFRESH_SHUTTER;
    }
  
    // update PPM outputs
    int periods[PPM_PHASES];
    periods[PPM_CHAN_PAN] = PPM_CENTER + model.servoPos.pan;
    periods[PPM_CHAN_TILT] = PPM_CENTER + model.servoPos.tilt;
    periods[PPM_CHAN_SHUTTER] = model.shutterServo;
    periods[PPM_CHAN_HOVER] = model.vertical ? HOVER_VERT_POS : HOVER_HOR_POS;
    periods[PPM_CHAN_UNUSED2] = PPM_CENTER;
    periods[PPM_CHAN_UNUSED3] = PPM_CENTER;
    ppmWrite(periods);
}

// ---------------------------------------------------------------------

void lcdSetup(void) 
{
    // start & clear the display
    display.begin();
    display.setRotation(0);
    
    // set text properties
    display.setTextSize(2);
    display.clearDisplay();

    // Display stuff that is only updated once.
    showBitmap(BM_CIRCLE, BLACK);

    display.refresh();
}

void setup()
{
    Serial.begin(9600);
    Serial.println(F("Hello!"));

    jsSetup();
    modelSetup();
    lcdSetup();

    ppmSetup();
}

void loop()
{
    if (ppm.startCycle) {
	ppm.startCycle = false;

	// Do 50 Hz stuff
	// read inputs
	jsPoll();

	// Update model (state)
	modelUpdate();

	// write outputs
	modelWrite();
	writeLcd();

        // Serial.println("yo.");
    }

    // Do as fast as possible.
}
