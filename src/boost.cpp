#include "boost.h"
#include "mbed.h"

PID::PID(float pterm, float iterm, float dterm, PinName p)
    : p_(pterm), i_(iterm), d_(dterm), perror_(0), pwm_(PwmOut(p)) {
  pwm_.write(0);
  pwm_.period_us(13);
  timer_.start();
}

float PID::duty(float desired, float now, float max) {
  float dt = std::chrono::duration_cast<std::chrono::microseconds>(
                 timer_.elapsed_time())
                 .count() /
             (float)3000000;
  float error = (now - desired) / max;
  integral_ += error * dt;
  float derivative = ((error - perror_) / (float)dt);
  float duty = (error * p_) + (integral_ * i_); // + derivative * d_);
#ifdef _PID
  printf("er: %.2f | des: %.2f | now: %.2f | du: %.2f\n", error, desired, now,
         duty);
#endif
  duty = (duty < 0) ? 0 : ((duty > 1) ? 1 : duty);
  perror_ = error;
  pwm_.write(duty);
  timer_.reset();
  timer_.start();
  return duty;
}

void PID::reset(void) {
  timer_.reset();
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
