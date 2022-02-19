#include "mbed.h"
#include "mppt.h"
#define SAMPLE_SIZE 5
#define CALC_INTERVAL 10
#define TRACKING_DELAY 20

static Mppt mppt;
static int po_cycle = CALC_INTERVAL;
static int tracking = TRACKING_DELAY;
static float vin[3] = {0,0,0};
static float iin[3] = {0,0,0};
static float duty[3] = {0,0,0};
static float max_duty[3] = {0,0,0};

void resetTracking(void) {
  memset(&vin, 0, 3*sizeof(float));
  memset(&iin, 0, 3*sizeof(float));
  po_cycle = CALC_INTERVAL;
}

int main() {
  while (mppt.notInit()) {
    printf("ERROR couldn't init MPPT\n");
    ThisThread::sleep_for(1s);
  }

  while (true) {
    if (tracking) {
      if (po_cycle < SAMPLE_SIZE) {
        vin[0] += mppt.bc1.getInputVoltage();
        vin[1] += mppt.bc2.getInputVoltage();
        vin[2] += mppt.bc3.getInputVoltage();
        iin[0] += mppt.bc1.getInputCurrent();
        iin[1] += mppt.bc2.getInputCurrent();
        iin[2] += mppt.bc3.getInputCurrent();
      }

      if (!po_cycle) {
        mppt.bc1.PO(vin[0],iin[0]);
        mppt.bc2.PO(vin[1],iin[1]);
        mppt.bc3.PO(vin[2],iin[2]);
        resetTracking();
      } else
        po_cycle--;

      duty[0] = mppt.bc1.pid.duty(mppt.bc1.getRefVoltage(), mppt.bc1.getInputVoltage());
      duty[1] = mppt.bc2.pid.duty(mppt.bc2.getRefVoltage(), mppt.bc2.getInputVoltage());
      duty[2] = mppt.bc3.pid.duty(mppt.bc3.getRefVoltage(), mppt.bc3.getInputVoltage());

      if (mppt.getOutputCurrent() > mppt.maxOutputCurrent.getValue()) {
        tracking--;
        if (!tracking) {
          max_duty[0] = duty[0];
          max_duty[1] = duty[1];
          max_duty[2] = duty[2];
          mppt.bc1.pid.reset();
          mppt.bc2.pid.reset();
          mppt.bc3.pid.reset();
        } else
          tracking = TRACKING_DELAY;
      }
    }

    else {
      float vout = mppt.getOutputVoltage();
      float share = mppt.maxOutputCurrent.getValue()/3;
      duty[0] = mppt.bc1.pid.duty(share, mppt.bc1.getInputCurrent()*mppt.bc1.getInputVoltage()/vout);
      duty[1] = mppt.bc2.pid.duty(share, mppt.bc2.getInputCurrent()*mppt.bc2.getInputVoltage()/vout);
      duty[2] = mppt.bc3.pid.duty(share, mppt.bc3.getInputCurrent()*mppt.bc3.getInputVoltage()/vout);
      if (max_duty[0] <= duty[0] ||
          max_duty[1] <= duty[1] ||
          max_duty[2] <= duty[2]) {
        resetTracking();
        tracking = TRACKING_DELAY; 
        mppt.bc1.pid.reset();
        mppt.bc2.pid.reset();
        mppt.bc3.pid.reset();
      }
    }
  }
}
