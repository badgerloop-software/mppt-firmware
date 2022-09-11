#include "boost.h"
#include "mbed.h"

PID::PID(float pterm, float iterm, float dterm, PinName p)
    : p_(pterm), i_(iterm), d_(dterm), perror_(0), pwm_(PwmOut(p)) {
  pwm_.write(0);
  pwm_.period_us(13);
}

float PID::duty(float desired, float now, float max, uint64_t current_time) {
  float dt = (current_time - p_time_) / (float)CYCLE_MS;
  float error = (now - desired) / max;
#ifdef _PID
  //printf("er: %.3f | des: %.3f | now: %.3f | du: %.3f | dt: %.9f\n", error,
  //       desired, now, duty_, dt);
  printf("duty: %.2f\n",duty_);
#endif
  integral_ += error * dt;
  float derivative = ((error - perror_) / (float)dt);
  static int tmp_duty = 0;
  duty_ = ((tmp_duty++) % 50) / (float)100; // + derivative * d_);
  duty_ = (duty_ < 0) ? 0 : ((duty_ > 1) ? 1 : duty_);
  perror_ = error;
  pwm_.write(duty_);
  p_time_ = current_time;
  return duty_;
}

void PID::reset(uint64_t current_time) {
  p_time_ = current_time;
  perror_ = 0;
  integral_ = 0;
}

BoostConverter::BoostConverter(PinName v, PinName i, PinName p)
    : voltageADC_(AnalogIn(v)), currentADC_(AnalogIn(i)),
      pid(PID(PTERM, ITERM, DTERM, p)), pp_(0), dir_(true) {}

float BoostConverter::getIin(void) { return currentADC_.read() * I_SCALE; }
//  Declan's Equation : read()*3.3*0.943+0.0294;
float BoostConverter::getVin(void) { return voltageADC_.read() * V_SCALE; }

float BoostConverter::PO(float vin, float iin) {
  float power = vin * iin;
  if (power < pp_) {
    dir_ = !dir_;
#ifdef _PO
    printf("  SWITCHED DIRECTION\n");
#endif
  }
  pp_ = power;
  return dir_ ? PO_VOLTAGE_STEP : -1 * PO_VOLTAGE_STEP;
}
