#pragma once

#define PPM_CHANNELS (6)

// TODO: Convert API value param from 0.5uS ticks to 1uS ticks

class Ppm
{
  public:
  // Constructor
  Ppm();
  
  void setup();

  // Set PPM Channel to value.
  // A value of zero corresponds to the center of throw, pulse width of 1500uS
  // A positive value, v, corresponds to a longer pulse
  void write(int chan, int value_us);

  // Wait until next PPM cycle starts
  void sync();
};
