#include "mbed.h"

#define DUTY .5
#define PIN PA_10

static PwmOut pwm = PwmOut(PIN);
int main(void) {
  pwm.period_us(13);
  pwm.write(DUTY);
  while (true) {}
  
}
