#include "mbed.h"

#define PO_VOLTAGE_STEP 0.1
#define PTERM 5
#define ITERM 5
#define DTERM 5

class PID {
private:
  unsigned char _p;
  unsigned char _i;
  unsigned char _d;
  long _ptime;
  float _integral;
  float _perror;
  PwmOut _pwm;
  Timer _timer;

public:
  float duty(float desired, float now);
  void reset(void);
  PID(unsigned char pterm, unsigned char iterm, unsigned char dterm, PinName p);
};

class BoostConverter {
private:
  float _pp;
  float _vref;
  bool _dir;
  AnalogIn _voltageADC;
  AnalogIn _currentADC;

public:
  PID pid;
  float getInputCurrent(void);
  float getInputVoltage(void);
  float getRefVoltage(void);
  void PO(float vin, float iin);
  BoostConverter(PinName v, PinName i, PinName p);
};
