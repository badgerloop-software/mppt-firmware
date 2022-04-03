#include "boost.h"
#include "mbed.h"

PID::PID(unsigned char pterm, unsigned char iterm, unsigned char dterm, PinName p)
    : _p(pterm), _i(iterm), _d(dterm), _perror(0), _pwm(PwmOut(p)) {
  _pwm.period_us(13);
  _timer.start();
}

float PID::duty(float desired, float now, float max) {
  std::chrono::duration<float, std::milli> dt = std::chrono::duration_cast<std::chrono::milliseconds>(_timer.elapsed_time());
  float error = (desired - now)/max;
  _integral += error * (float)dt.count();
  float duty = 1 - (error * _p + _integral * _i + ((error - _perror) / (float)dt.count()) * _d);
  printf("duty set to %.6f\n",duty);
  if (duty < 0 || duty > 1) {
    printf("ERROR DUTY OUT OF RANGE!\n");
    duty = 1;
  }
  _perror = error;
  _pwm.write(duty);
  _timer.reset();
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

float BoostConverter::getIin(void) {
  return _currentADC.read_voltage()*V_SCALE;
}
float BoostConverter::getVin(void) {
  return _voltageADC.read_voltage()*I_SCALE;
}

float BoostConverter::PO(float vin, float iin) {
  float power = vin * iin;
  if (power < _pp)
    _dir ^= 1;
  _pp = power;
  return _dir ? PO_VOLTAGE_STEP : -1*PO_VOLTAGE_STEP;
}
