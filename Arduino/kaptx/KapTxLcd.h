// #include <Adafruit_SharpMem.h>

class KapTxModel;

class KapTxLcd
{
  public:
    KapTxLcd(int sckPin, int mosiPin, int ssPin);

  private:
    // Instance data
    Adafruit_SharpMem display;

  public:
    // Public API
    void setup();
    void update(KapTxModel *model);

  private:
    // Utility methods
    void showBitmap(unsigned char bitmapId, unsigned char color);
    void showShutter(ShutterState_t state);  // based on showShutter
    void showShootMode(ShootMode_t mode);    // based on showShootMode
    void showShots(int shots);               // based on showShots
    void showHoVer(bool vertical);           // based on showHoVer
    void showPan(int pan);
    void showTilt(int tilt);
};
