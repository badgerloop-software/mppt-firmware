#include "mbed.h"
#include "mppt.h"

static Mppt mppt;
static int calc_count;
static float vin[3]; // for bc 1,2, and 3
static float iin[3];
static float duty_max[3];
static int tracking = TRACKING_DELAY;


void resetTracking(void) {
  memset(&vin, 0, 3*sizeof(float));
  memset(&iin, 0, 3*sizeof(float));
  calc_count = CALC_INTERVAL;
}

int main() {
  resetTracking();

  while (mppt.notInit()) {
    printf("ERROR couldn't init MPPT\n");
    ThisThread::sleep_for(1s);
  }

  while (true) {
    if (tracking) {
      /*
       * Only start summing the
       * last SAMPLE_SIZE samples
       */
      if (calc_count < SAMPLE_SIZE) {
        vin[0] += mppt.bc1.getInputVoltage();
        vin[1] += mppt.bc2.getInputVoltage();
        vin[2] += mppt.bc3.getInputVoltage();

        iin[0] += mppt.bc1.getInputCurrent();
        iin[1] += mppt.bc2.getInputCurrent();
        iin[2] += mppt.bc3.getInputCurrent();
      }

      if (!calc_count) {
        mppt.bc1.PO(vin[0],iin[0]);
        mppt.bc2.PO(vin[1],iin[1]);
        mppt.bc3.PO(vin[2],iin[2]);
        resetTracking();
      }
      mppt.bc1.PID();
      mppt.bc2.PID();
      mppt.bc3.PID();

      if (mppt.getOutputVoltage() > mppt.maxOutputVoltage.getValue() ||
          (mppt.getOutputCurrent()) > mppt.maxOutputCurrent.getValue()) {
        tracking--;
        duty_max[0] = mppt.bc2.getDuty();
        duty_max[1] = mppt.bc2.getDuty();
        duty_max[2] = mppt.bc3.getDuty();
        if (!tracking) resetTracking();
      } else
        tracking = TRACKING_DELAY;
    }

    else {
      if (mppt.mode.getValue()) mppt.VPID();
      else mppt.IPID();
      if (duty_max[0] >= mppt.bc1.getDuty() ||
          duty_max[1] >= mppt.bc2.getDuty() ||
          duty_max[2] >= mppt.bc3.getDuty())
        tracking = TRACKING_DELAY;
    }
  }
}
