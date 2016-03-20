#pragma once


class Model;

class View
{
  public:
  View(Model *model);

  private:
    // Instance data
    // Adafruit_SharpMem display;
    Model *model;
    unsigned char sinceRefresh;
    bool refreshNeeded;

  public:
    // Public API
    void setup();
    void update();

  private:
    // Utility methods
    void showBitmap(unsigned char bitmapId, unsigned char color);
    void showShutter(unsigned char state);   // based on showShutter
    void showShootMode(unsigned char mode, bool inv);  // based on showShootMode
    void showShots(bool autokap, int shots, bool inv);               // based on showShots
    void showHoVer(bool vertical, bool inv);           // based on showHoVer
    void showPan(int pan);
    void showTilt(int tilt);
    void drawUpdates(unsigned char flags);
};
