#include "mbed.h"
#include "mppt.h"
#define SAMPLE_SIZE 5
#define PO_DELAY (15 * SAMPLE_SIZE)
#define TRACKING_DELAY 10
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
static float vref[3] = {30, 30, 30};
static float vin[3] = {0, 0, 0};
static float iin[3] = {0, 0, 0};
static float vout = 0;
void readADC(void) {
  vout = mppt.getVout();
#ifdef _SIMULATION
  switch (_SIMULATION) {
  case 0:
    iin[0] = mppt.bc0.getIin();
    vin[0] = mppt.bc0.getVin();
    break;
  case 1:
    iin[1] = mppt.bc1.getIin();
    vin[1] = mppt.bc1.getVin();
    break;
  case 2:
    iin[2] = mppt.bc2.getIin();
    vin[2] = mppt.bc2.getVin();
    break;
  }
#else
  iin[0] = mppt.bc0.getIin();
  vin[0] = mppt.bc0.getVin();
  iin[1] = mppt.bc1.getIin();
  vin[1] = mppt.bc1.getVin();
  iin[2] = mppt.bc2.getIin();
  vin[2] = mppt.bc2.getVin();
#endif

#ifdef _ADC
  printf("/ / / / / / / READ ADC  / / / / / / / \n");
  printf(" / / / / / / CYCLE %llu / / / / / \n", cycle_count++);
#ifdef _SIMULATION
  for (int i = _SIMULATION; i <= _SIMULATION; i++) {
#else
  for (int i = 0; i < 3; i++) {
#endif
    printf("  iin[%d]: %.3f  vin[%d]: %.3f\n", i, iin[i], i, vin[i]);
  }
  printf("            vout: %.3f\n", vout);
  printf("/ / / / / / / / / / / / / / / / / / \n\n");
#endif
}
void resetPO(void) {
  memset(&sample_vin, 0, 3 * sizeof(float));
  memset(&sample_iin, 0, 3 * sizeof(float));
  po = PO_DELAY;
}
void resetPID(uint64_t current_time) {
  mppt.bc0.pid.reset(current_time);
  mppt.bc1.pid.reset(current_time);
  mppt.bc2.pid.reset(current_time);
}

uint64_t get_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             Kernel::Clock::now().time_since_epoch())
      .count();
}

int main(void) {
  while (mppt.notInit() || mppt.maxIout.getValue() == -1) {
    printf("No Message...\n");
    ThisThread::sleep_for(2s);
  }

  uint64_t current_time = get_ms();
  resetPID(current_time);

  while (true) {
    thread_sleep_until(current_time + CYCLE_MS);
    current_time = get_ms();
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
#ifdef _SIMULATION
        switch (_SIMULATION) {
        case 0:
          vref[0] += mppt.bc0.PO(sample_vin[0], sample_iin[0]);
          break;
        case 1:
          vref[1] += mppt.bc1.PO(sample_vin[1], sample_iin[1]);
          break;
        case 2:
          vref[2] += mppt.bc2.PO(sample_vin[2], sample_iin[2]);
          break;
        }
#else
        vref[0] += mppt.bc0.PO(sample_vin[0], sample_iin[0]);
        vref[1] += mppt.bc1.PO(sample_vin[1], sample_iin[1]);
        vref[2] += mppt.bc2.PO(sample_vin[2], sample_iin[2]);
#endif

#ifdef _PO
        printf("P&O:\n");
        for (int i = 0; i < 3; i++) {
          printf("| vref[%d]: %.3f\n", i, vref[i]);
        }
#endif
        resetPO();
      } else
        po--;

#ifdef _SIMULATION
      switch (_SIMULATION) {
      case 0:
        duty[0] = mppt.bc0.pid.duty(vref[0], vin[0], MAXV, current_time);
        break;
      case 1:
        duty[1] = mppt.bc1.pid.duty(vref[1], vin[1], MAXV, current_time);
        break;
      case 2:
        duty[2] = mppt.bc2.pid.duty(vref[2], vin[2], MAXV, current_time);
        break;
      }
#else
      duty[0] = mppt.bc0.pid.duty(vref[0], vin[0], MAXV, current_time);
      duty[1] = mppt.bc1.pid.duty(vref[1], vin[1], MAXV, current_time);
      duty[2] = mppt.bc2.pid.duty(vref[2], vin[2], MAXV, current_time);
#endif

      float iout =
          (iin[0] + iin[1] + iin[2]) * (vin[0] + vin[1] + vin[2]) / vout;
      printf("VOUT: %f\tIOUT: %f\n",vout,iout);
#ifdef _TRACKING
      printf("*-*-*-*-*-*-* SET DUTY -*-*-*-*-*-*-*\n");
      printf("*-*-*-*-*- MODE: TRACKING *-*-*-*-*-*\n");
#ifdef _SIMULATION
      for (int i = _SIMULATION; i <= _SIMULATION; i++) {
#else
      for (int i = 0; i < 3; i++) {
#endif
        printf("  duty[%d]: %.3f\n", i, duty[i]);
      }
      printf("            iout: %.3f             \n", iout);
      printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
#endif
      if (iout > mppt.maxIout.getValue()) {
        printf("MAX CURRENT EXCEEDED\n");
        tracking--;
        if (!tracking) {
          p_duty[0] = duty[0];
          p_duty[1] = duty[1];
          p_duty[2] = duty[2];
          resetPID(current_time);
          resetPO();
        }
      } else
        tracking = TRACKING_DELAY;
    }

    else {

#ifdef _SIMULATION
      float iout_share = mppt.maxIout.getValue();
      switch (_SIMULATION) {
      case 0:
        duty[0] = mppt.bc0.pid.duty(iout_share, iin[0] * vin[0] / vout, MAXI,
                                    current_time);
        break;

      case 1:
        duty[1] = mppt.bc1.pid.duty(iout_share, iin[1] * vin[1] / vout, MAXI,
                                    current_time);
        break;

      case 2:
        duty[2] = mppt.bc2.pid.duty(iout_share, iin[2] * vin[2] / vout, MAXI,
                                    current_time);
        break;
      }
#else
      float iout_share = mppt.maxIout.getValue() / 3;
      duty[0] = mppt.bc0.pid.duty(iout_share, iin[0] * vin[0] / vout, MAXI,
                                  current_time);
      duty[1] = mppt.bc1.pid.duty(iout_share, iin[1] * vin[1] / vout, MAXI,
                                  current_time);
      duty[2] = mppt.bc2.pid.duty(iout_share, iin[2] * vin[2] / vout, MAXI,
                                  current_time);
#endif

#ifdef _CURRENT
      printf("v^v^v^v^v^v^v SET DUTY ^v^v^v^v^v^v^v\n");
      printf("v^v^v^v^v^ MODE: CURRENT ^v^v^v^v^v^v\n");
#ifdef _SIMULATION
      for (int i = _SIMULATION; i <= _SIMULATION; i++) {
#else
      for (int i = 0; i < 3; i++) {
#endif
        printf("  duty[%d]: %.3f\n", i, duty[i]);
      }
      printf("            iout_share: %.3f\n", iout_share);
      printf("v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v^v\n\n");
#endif
      if (p_duty[0] < duty[0] || p_duty[1] < duty[1] || p_duty[2] < duty[2]) {
#ifdef _CURRENT
        printf("  DUTY EXCEEDED PREVIOUS, CHANGING TO TRACKING\n");
#endif
        tracking = TRACKING_DELAY;
        resetPID(current_time);
        resetPO();
      }
    }
  }
}
