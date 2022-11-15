#include "mbed.h"
#include "mppt.h"
#define SAMPLE_SIZE 5
#define PO_DELAY (15 * SAMPLE_SIZE)
#define TRACKING_DELAY 10
#define MAXV 60
#define MAXI 7
#define MPP_VIN_CYCLES 500

typedef std::chrono::duration<long long, std::ratio<1, 1000>> millisecond;

static Mppt mppt;
static int po = PO_DELAY;
static unsigned long long cycles = 0;
static int tracking = TRACKING_DELAY;
static float sample_vin[3] = {0, 0, 0};
static float sample_iin[3] = {0, 0, 0};
static float p_duty[3] = {0, 0, 0};
static float duty[3] = {0, 0, 0};
static float vref[3] = {30, 30, 30};
static float vin[3] = {0, 0, 0};
static float iin[3] = {0, 0, 0};
static float vout = 0;

void readADC() {
  vout = mppt.getVout();
#ifdef SIMULATION_
  switch (SIMULATION_) {
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
}

void resetPO() {
  memset(&sample_vin, 0, 3 * sizeof(float));
  memset(&sample_iin, 0, 3 * sizeof(float));
  po = PO_DELAY;
}

void resetPID() {
  mppt.bc0.pid.reset();
  mppt.bc1.pid.reset();
  mppt.bc2.pid.reset();
}

millisecond get_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      Kernel::Clock::now().time_since_epoch());
}

#ifdef SIMULATION_
float mpp_vin() {
  float mpp_vin, mpp_pow = 0;
  for (int i = 0; i <= 20; i++) {
    switch (SIMULATION_) {
    case 0:
      mppt.bc0.pid.pwm_.write(i * (float)1 / 20.05);
      break;
    case 1:
      mppt.bc1.pid.pwm_.write(i * .05);
      break;
    case 2:
      mppt.bc2.pid.pwm_.write(i * .05);
      break;
    }
    // ThisThread::sleep_for(CYCLE_MS);
    float vin, iin, pow;
    switch (SIMULATION_) {
    case 0:
      vin = mppt.bc0.getVin();
      iin = mppt.bc0.getIin();
      break;
    case 1:
      vin = mppt.bc1.getVin();
      iin = mppt.bc1.getIin();
      break;
    case 2:
      vin = mppt.bc2.getVin();
      iin = mppt.bc2.getIin();
      break;
    }
    pow = vin * iin;
    if (pow > mpp_pow) {
      mpp_vin = vin;
      mpp_pow = pow;
    }
  }
  switch (SIMULATION_) {
  case 0:
    mppt.bc0.pid.pwm_.write(duty[0]);
    break;
  case 1:
    mppt.bc1.pid.pwm_.write(duty[1]);
    break;
  case 2:
    mppt.bc2.pid.pwm_.write(duty[2]);
    break;
  }
  return mpp_vin;
}
#endif

int main() {
  while (mppt.notInit() || mppt.maxIout.getValue() == -1) {
    printf("No Message...\n");
    ThisThread::sleep_for(2s);
  }

  millisecond current_time, p_time = get_ms();

  while (true) {
    thread_sleep_until((p_time + CYCLE_MS).count());
    current_time = get_ms();
#ifdef SIMULATION_
    if (cycles++ % MPP_VIN_CYCLES == 0)
      printf("MPP VIN: %f | ", mpp_vin());
#endif
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
#ifdef SIMULATION_
        switch (SIMULATION_) {
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

        resetPO();
      } else
        po--;

#ifdef SIMULATION_
      switch (SIMULATION_) {
      case 0:
        duty[0] =
            mppt.bc0.pid.duty(vref[0], vin[0], MAXV, (current_time-p_time).count());
        break;
      case 1:
        duty[1] =
            mppt.bc1.pid.duty(vref[1], vin[1], MAXV, (current_time-p_time).count());
        break;
      case 2:
        duty[2] =
            mppt.bc2.pid.duty(vref[2], vin[2], MAXV, (current_time-p_time).count());
        break;
      }
#else
      duty[0] = mppt.bc0.pid.duty(vref[0], vin[0], MAXV, (current_time-p_time).count());
      duty[1] = mppt.bc1.pid.duty(vref[1], vin[1], MAXV, (current_time-p_time).count());
      duty[2] = mppt.bc2.pid.duty(vref[2], vin[2], MAXV, (current_time-p_time).count());
#endif

      float iout =
          (iin[0] + iin[1] + iin[2]) * (vin[0] + vin[1] + vin[2]) / vout;
      if (iout > mppt.maxIout.getValue()) {
        printf("MAX CURRENT EXCEEDED\n");
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

#ifdef SIMULATION_
      float iout_share = mppt.maxIout.getValue();
      switch (SIMULATION_) {
      case 0:
        duty[0] = mppt.bc0.pid.duty(iout_share, iin[0] * vin[0] / vout, MAXI,
                                    (current_time-p_time).count());
        break;

      case 1:
        duty[1] = mppt.bc1.pid.duty(iout_share, iin[1] * vin[1] / vout, MAXI,
                                    (current_time-p_time).count());
        break;

      case 2:
        duty[2] = mppt.bc2.pid.duty(iout_share, iin[2] * vin[2] / vout, MAXI,
                                    (current_time-p_time).count());
        break;
      }
#else
      float iout_share = mppt.maxIout.getValue() / 3;
      duty[0] = mppt.bc0.pid.duty(iout_share, iin[0] * vin[0] / vout, MAXI,
                                  (current_time-p_time).count());
      duty[1] = mppt.bc1.pid.duty(iout_share, iin[1] * vin[1] / vout, MAXI,
                                  (current_time-p_time).count());
      duty[2] = mppt.bc2.pid.duty(iout_share, iin[2] * vin[2] / vout, MAXI,
                                  (current_time-p_time).count());
#endif

      if (p_duty[0] < duty[0] || p_duty[1] < duty[1] || p_duty[2] < duty[2]) {
        tracking = TRACKING_DELAY;
        resetPID();
        resetPO();
      }
    }
    p_time = current_time;
  }
}
