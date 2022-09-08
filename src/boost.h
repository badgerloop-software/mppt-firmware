#include "mbed.h"

#define PO_VOLTAGE_STEP .5 
#define PTERM 0.1
#define ITERM 0.3
#define DTERM 0.1

/*
HIDDEN DEBUG:

#define _TRACKING
#define _CURRENT
#define _PO
#define _ADC
*/
#define _PID
#define _SIMULATION

constexpr float V_SCALE = (103.3) / 3.3 * 3.3;
constexpr float I_SCALE = 1 / (100 * .004) * 3.3;

class PID {
private:
  float _p;
  float _i;
  float _d;
  float _integral;
  float _perror;
  PwmOut _pwm;
  Timer _timer;

public:
  float duty(float desired, float now, float max);
  void reset(void);
  PID(float pterm, float iterm, float dterm, PinName p);
};

class BoostConverter {
private:
  float _pp;
  bool _dir;
  AnalogIn _voltageADC;
  AnalogIn _currentADC;

public:
  PID pid;
  float getIin(void);
  float getVin(void);
  float PO(float vin, float iin);
  BoostConverter(PinName v, PinName i, PinName p);
};
