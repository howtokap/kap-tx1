#pragma once

class Joystick
{
 public:
  Joystick(int xPin, int yPin, int butPin);

  void setup();
  void poll();

  int getIndex24();
  int getIndex16();
  int getIndex16_offs();

  bool wasPressed();
  bool isOut();
  bool isCenter();
  
 private:
  int x_pin;
  int y_pin; 
  int but_pin;

 public:
  int x;
  int y;
  bool but;
  bool pressed;
  unsigned int debounce;
};
