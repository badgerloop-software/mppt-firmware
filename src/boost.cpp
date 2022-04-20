#include "boost.h"
#include "mbed.h"

PID::PID(unsigned char pterm, unsigned char iterm, unsigned char dterm,
         PinName p)
    : _p(pterm), _i(iterm), _d(dterm), _perror(0), _pwm(PwmOut(p)) {
  _pwm.write(0);
  _pwm.period_us(13);
  _timer.start();
}

float PID::duty(float desired, float now, float max) {
  std::chrono::duration<float> dt =
      std::chrono::duration_cast<std::chrono::seconds>(_timer.elapsed_time());
  float error = (desired - now) / max;
  _integral += error * (float)dt.count();
  float derivative = ((error - _perror) / (float)dt.count());
  float duty = (error * _p + _integral * _i + derivative * _d);
#ifdef _PID
  printf("@%@%@%@%@%@%@ PID @%@%@%@%@%@%@%\n");
  printf("    desired: %.3f\n", desired);
  printf("        now: %.3f\n\n", now);
  printf("         dt: %.3f\n", dt.count());
  printf("      error: %.3f\n", error);
  printf("   integral: %.3f\n", _integral);
  printf(" derivative: %.3f\n\n", derivative);

  printf("       duty: %.3f\n", duty);
  printf("@%@%@%@%@%@%@%@%@%@%@%@%@%@%@%@%\n");
#endif
  if (duty < 0 || duty > 1) {
    printf("ERROR DUTY OUT OF RANGE! %.3f\n", duty);
  }
  _perror = error;
#ifdef _SIMULATION
  _pwm.write(0);
#else
  _pwm.write(duty);
#endif
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

float BoostConverter::getIin(void) { return _currentADC.read() * I_SCALE; }
  //  Declan's Equation : read()*3.3*0.943+0.0294;
float BoostConverter::getVin(void) { return _voltageADC.read() * V_SCALE; }

float BoostConverter::PO(float vin, float iin) {
  float power = vin * iin;
  if (power < _pp) {
    _dir ^= 1;
#ifdef _PO
    printf("  SWITCHED DIRECTION\n");
#endif
  }
  _pp = power;
  return _dir ? PO_VOLTAGE_STEP : -1 * PO_VOLTAGE_STEP;
}
