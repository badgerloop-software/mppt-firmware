#include "mbed.h"
#include <FastPWM.h>

#define CYCLE_MS 100ms
#define PO_VOLTAGE_STEP .5
#define PTERM 0.4
#define ITERM 0.2
#define DTERM 0.0

/*
HIDDEN DEBUG:
*/
#define SIMULATION_ 1
#define PID_

constexpr float V_SCALE = (103.3) / 3.3 * 3.3;
constexpr float I_SCALE = 1 / (100 * .004) * 3.3;

class PID {
private:
  float p_;
  float i_;
  float d_;
  float integral_;
  float perror_;
  uint64_t p_time_;

public:
  float duty(float desired, float now, float max, uint64_t current_time);
  void reset(uint64_t current_time);
  PID(float pterm, float iterm, float dterm, PinName p);
  FastPWM pwm_;
};

class BoostConverter {
private:
  float pp_;
  bool dir_;
  AnalogIn voltageADC_;
  AnalogIn currentADC_;

public:
  PID pid;
  float getIin(void);
  float getVin(void);
  float PO(float vin, float iin);
  BoostConverter(PinName v, PinName i, PinName p);
};
