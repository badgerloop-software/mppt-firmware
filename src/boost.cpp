#include "boost.h"
#include "mbed.h"

PID::PID(float pterm, float iterm, float dterm,
         PinName p)
    : _p(pterm), _i(iterm), _d(dterm), _perror(0), _pwm(PwmOut(p)) {
  _pwm.write(0);
  _pwm.period_us(13);
  _timer.start();
}

float PID::duty(float desired, float now, float max) {
  float dt = std::chrono::duration_cast<std::chrono::microseconds>(_timer.elapsed_time()).count()/(float)1000000;
  float error = (now-desired) / max;
  _integral += error * dt;
  float derivative = ((error - _perror) / (float)dt);
  float duty = (error * _p) + (_integral * _i); // + derivative * _d);
#ifdef _PID
  printf("er: %.2f | des: %.2f | now: %.2f | du: %.2f\n", error, desired, now, duty);
#endif
  duty = (duty < 0) ? 0 : ((duty > 1) ? 1 : duty);
  _perror = error;
  _pwm.write(duty);
  _timer.reset();
  _timer.start();
  return duty;
}

void PID::reset(void) {
  _timer.reset();
  _perror = 0;
  _integral = 0;
}

BoostConverter::BoostConverter(PinName v, PinName i, PinName p)
    : _voltageADC(AnalogIn(v)), _currentADC(AnalogIn(i)),
      pid(PID(PTERM, ITERM, DTERM, p)), _pp(0), _dir(true) {}

float BoostConverter::getIin(void) { return _currentADC.read() * I_SCALE; }
  //  Declan's Equation : read()*3.3*0.943+0.0294;
float BoostConverter::getVin(void) { return _voltageADC.read() * V_SCALE; }

float BoostConverter::PO(float vin, float iin) {
  float power = vin * iin;
  if (power < _pp) {
    _dir = !_dir;
#ifdef _PO
    printf("  SWITCHED DIRECTION\n");
#endif
  }
  _pp = power;
  return _dir ? PO_VOLTAGE_STEP : -1 * PO_VOLTAGE_STEP;
}
