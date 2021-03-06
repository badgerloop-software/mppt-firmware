#include "mbed.h"

#define PO_VOLTAGE_STEP 0.1
#define PTERM 1
#define ITERM 0
#define DTERM 0

/*
DEBUG
*/
#define _PID
#define _PO
#define _TRACKING
#define _CURRENT
#define _INIT
#define _ADC

constexpr float V_SCALE = (103.3)/3.3 * 3.3;
constexpr float I_SCALE = 1/(100*.004) * 3.3;

class PID {
private:
  unsigned char _p;
  unsigned char _i;
  unsigned char _d;
  float _integral;
  float _perror;
  PwmOut _pwm;
  Timer _timer;

public:
  float duty(float desired, float now, float max);
  void reset(void);
  PID(unsigned char pterm, unsigned char iterm, unsigned char dterm, PinName p);
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
