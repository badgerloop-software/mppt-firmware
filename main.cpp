#include "mbed.h"
#include "mppt.h"
#define SAMPLE_SIZE 5
#define PO_DELAY 10
#define TRACKING_DELAY 20

static Mppt mppt;
static int po = PO_DELAY;
static int tracking = TRACKING_DELAY;
static float sample_vin[3] = {0,0,0};
static float sample_iin[3] = {0,0,0};
static float p_duty[3] = {0,0,0};
static float duty[3] = {0,0,0};
static float vref[3] = {48,48,48};
static float vin[3] = {0,0,0};
static float iin[3] = {0,0,0};
static float vout = 0;
inline void readADC(void) {
  vout = mppt.getVout();
  iin[0] = mppt.bc1.getIin();
  iin[1] = mppt.bc2.getIin();
  iin[2] = mppt.bc3.getIin();
  vin[0] = mppt.bc1.getVin();
  vin[1] = mppt.bc2.getVin();
  vin[2] = mppt.bc3.getVin();
} inline void resetPO(void) {
  memset(&sample_vin, 0, 3*sizeof(float));
  memset(&sample_iin, 0, 3*sizeof(float));
  po = PO_DELAY;
} inline void resetPID(void) {
  mppt.bc1.pid.reset();
  mppt.bc2.pid.reset();
  mppt.bc3.pid.reset();
}

int main(void) {
  while (mppt.notInit() || mppt.maxIout.getValue() == -1) {
    ThisThread::sleep_for(2s);
  }

  while (true) {
    readADC();
    if (tracking) {
      if (po < SAMPLE_SIZE) {
        sample_vin[0] += vin[0];
        sample_vin[1] += vin[1];
        sample_vin[2] += vin[2];

        sample_iin[0] += iin[0];
        sample_iin[1] += iin[1];
        sample_iin[2] += iin[2];
      }

      if (!po) {
        vref[0] += mppt.bc1.PO(sample_vin[0],sample_iin[0]);
        vref[1] += mppt.bc2.PO(sample_vin[1],sample_iin[1]);
        vref[2] += mppt.bc3.PO(sample_vin[2],sample_iin[2]);
        resetPO();
      } else
        po--;

      duty[0] = mppt.bc1.pid.duty(vref[0], vin[0]);
      duty[1] = mppt.bc2.pid.duty(vref[1], vin[1]);
      duty[2] = mppt.bc3.pid.duty(vref[2], vin[2]);

      float iout = (vin[0]+iin[1]+iin[2])*(vin[0]+vin[1]+vin[2])/vout;

      if (iout > mppt.maxIout.getValue()) {
        tracking--;
        if (!tracking) {
          p_duty[0] = duty[0];
          p_duty[1] = duty[1];
          p_duty[2] = duty[2];
          resetPID();
          resetPO();
        }
      } else 
        tracking = TRACKING_DELAY;
    }

    else {

      float iout_share = mppt.maxIout.getValue()/3;

      duty[0] = mppt.bc1.pid.duty(iout_share, iin[0]*vin[0]/vout);
      duty[1] = mppt.bc2.pid.duty(iout_share, iin[1]*vin[1]/vout);
      duty[2] = mppt.bc3.pid.duty(iout_share, iin[2]*vin[2]/vout);
      if (p_duty[0] <= duty[0] ||
          p_duty[1] <= duty[1] ||
          p_duty[2] <= duty[2]) {
        tracking = TRACKING_DELAY; 
        resetPID();
        resetPO();
      }
    }
  }
}
