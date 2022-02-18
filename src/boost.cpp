#include "boost.h"
#include "mbed.h"

PID::PID(unsigned char pterm, unsigned char iterm, unsigned char dterm, PinName p)
    : _p(pterm), _i(iterm), _d(dterm), _perror(0), _pwm(PwmOut(p)) {
  _pwm.period_us(13);
  _timer.start();
}

float PID::duty(float desired, float now) {
  long time = duration_cast<std::chrono::milliseconds>(_timer.elapsed_time()).count();
  float dt = time - _ptime;
  _ptime = time;
  float error = desired - now;
  float derivative = (error - _perror) / dt;
  _perror = error;
  _integral += error * dt;
  float duty = error * _p + _integral * _i + derivative * _d;
  _pwm.write(duty);
  return duty;
}

void PID::reset(void) {
  _timer.reset();
  _perror = 0;
  _integral = 0;
}

BoostConverter::BoostConverter(PinName v, PinName i, PinName p)
    : _voltageADC(AnalogIn(v)), _currentADC(AnalogIn(i)),
      pid(PID(PTERM, ITERM, DTERM, p)), _vref(48), _pp(0), _dir(true) {}

float BoostConverter::getInputCurrent(void) {
  return _currentADC.read_voltage();
}
float BoostConverter::getInputVoltage(void) {
  return _voltageADC.read_voltage();
}

float BoostConverter::getRefVoltage(void) { return _vref; }

void BoostConverter::PO(float vin, float iin) {
  float power = vin * iin;
  if (power < _pp)
    _dir ^= 1;
  if (_dir)
    _vref += PO_VOLTAGE_STEP;
  else
    _vref -= PO_VOLTAGE_STEP;
  _pp = power;
}