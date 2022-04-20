#include "mbed.h"
#include "mppt.h"
#define SAMPLE_SIZE 5
#define PO_DELAY 10
#define TRACKING_DELAY 20
#define MAXV 60
#define MAXI 7

static Mppt mppt;
static int po = PO_DELAY;
static unsigned long long cycle_count = 0;
static int tracking = TRACKING_DELAY;
static float sample_vin[3] = {0, 0, 0};
static float sample_iin[3] = {0, 0, 0};
static float p_duty[3] = {0, 0, 0};
static float duty[3] = {0, 0, 0};
static float vref[3] = {48, 48, 48};
static float vin[3] = {0, 0, 0};
static float iin[3] = {0, 0, 0};
static float vout = 0;
inline void readADC(void) {
  vout = mppt.getVout();
  iin[0] = mppt.bc1.getIin();
  vin[0] = mppt.bc1.getVin();
#ifndef _SIMULATION
  iin[1] = mppt.bc2.getIin();
  iin[2] = mppt.bc3.getIin();
  vin[1] = mppt.bc2.getVin();
  vin[2] = mppt.bc3.getVin();
#endif

#ifdef _ADC
  printf("/ / / / / / / READ ADC  / / / / / / / \n");
  printf(" / / / / / / CYCLE %llu / / / / / \n", cycle_count++);
#ifdef _SIMULATION
  for (int i = 0; i < 1; i++) {
#else
  for (int i = 0; i < 3; i++) {
#endif
    printf("  iin[%d]: %.3f  vin[%d]: %.3f\n", i, iin[i], i, vin[i]);
  }
  printf("            vout: %.3f\n", vout);
  printf("/ / / / / / / / / / / / / / / / / / \n\n");
#endif
}
inline void resetPO(void) {
  printf("#############\n   RESET PO   \n#############\n");
  memset(&sample_vin, 0, 3 * sizeof(float));
  memset(&sample_iin, 0, 3 * sizeof(float));
  po = PO_DELAY;
}
inline void resetPID(void) {
  printf("!!!!!!!!!!!!!\n   RESET PID  \n!!!!!!!!!!!!!\n");
  mppt.bc1.pid.reset();
  mppt.bc2.pid.reset();
  mppt.bc3.pid.reset();
}

int main(void) {
  while (mppt.notInit() || mppt.maxIout.getValue() == -1) {
    printf("No Message\n");
    ThisThread::sleep_for(2s);
  }
  // ThisThread::sleep_for(10s);

#ifdef _INIT
  printf("\n\nSUCCESSSSSSS!!!!\n  MAX IOUT: %.3f\n\n\n",
         mppt.maxIout.getValue());
#endif

  while (true) {
    ThisThread::sleep_for(4s);
    readADC();
    if (tracking) {
      if (po < SAMPLE_SIZE) {
#ifdef _PO
        printf("  COLLECTING VIN SAMPLE No.%d\n", SAMPLE_SIZE - po);
#endif
        sample_vin[0] += vin[0];
        sample_vin[1] += vin[1];
        sample_vin[2] += vin[2];

        sample_iin[0] += iin[0];
        sample_iin[1] += iin[1];
        sample_iin[2] += iin[2];
      }

      if (!po) {
        vref[0] += mppt.bc1.PO(sample_vin[0], sample_iin[0]);
#ifndef _SIMULATION
        vref[1] += mppt.bc2.PO(sample_vin[1], sample_iin[1]);
        vref[2] += mppt.bc3.PO(sample_vin[2], sample_iin[2]);
#endif

#ifdef _PO
        printf("  P&O:\n");
        for (int i = 0; i < 3; i++) {
          printf("  | vref[%d]: %.3f\n", i, vref[i]);
        }
#endif
        resetPO();
      } else
        po--;

      duty[0] = mppt.bc1.pid.duty(vref[0], vin[0], MAXV);
#ifndef _SIMULATION
      duty[1] = mppt.bc2.pid.duty(vref[1], vin[1], MAXV);
      duty[2] = mppt.bc3.pid.duty(vref[2], vin[2], MAXV);
#endif

      float iout =
          (iin[0] + iin[1] + iin[2]) * (vin[0] + vin[1] + vin[2]) / vout;
#ifdef _TRACKING
      printf("*-*-*-*-*-*-* SET DUTY -*-*-*-*-*-*-*\n");
      printf("*-*-*-*-*- MODE: TRACKING *-*-*-*-*-*\n");
#ifdef _SIMULATION
      for (int i = 0; i < 3; i++) {
#else
      for (int i = 0; i < 1; i++) {
#endif
        printf("  duty[%d]: %.3f\n", i, duty[i]);
      }
      printf("            iout: %.3f             \n", iout);
      printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
#endif
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

#ifdef _SIMULATION
      float iout_share = mppt.maxIout.getValue();
#else
      float iout_share = mppt.maxIout.getValue() / 3;
#endif

      duty[0] = mppt.bc1.pid.duty(iout_share, iin[0] * vin[0] / vout, MAXI);
#ifndef _SIMULATION
      duty[1] = mppt.bc2.pid.duty(iout_share, iin[1] * vin[1] / vout, MAXI);
      duty[2] = mppt.bc3.pid.duty(iout_share, iin[2] * vin[2] / vout, MAXI);
#endif

#ifdef _CURRENT
      printf("v^v^v^v^v^v^v SET DUTY ^v^v^v^v^v^v^v\n");
      printf("v^v^v^v^v^ MODE: CURRENT ^v^v^v^v^v^v\n");
#ifdef _SIMULATION
      for (int i = 0; i < 1; i++) {
#else
      for (int i = 0; i < 3; i++) {
#endif
        printf("  duty[%d]: %.3f\n", i, duty[i]);
      }
      printf("            iout_share: %.3f\n", iout_share);
      printf("v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v\n\n");
#endif
      if (p_duty[0] < duty[0] || p_duty[1] < duty[1] ||
          p_duty[2] < duty[2]) {
#ifdef _CURRENT
        printf("  DUTY EXCEEDED PREVIOUS, CHANGING TO TRACKING\n");
#endif
        tracking = TRACKING_DELAY;
        resetPID();
        resetPO();
      }
    }
  }
}
