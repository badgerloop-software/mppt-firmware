#include "mbed.h"
#define MPPT_BASE_ID 0x0000
#define MPPT_MODE_ID MPPT_BASE_ID + 8 // mode
#define MPPT_MOV_ID MPPT_BASE_ID + 10 // max output voltage
#define MPPT_MOC_ID MPPT_BASE_ID + 11 // max output current

#define SAMPLE_SIZE 5
#define CALC_INTERVAL 10
#define TRACKING_DELAY 20
#define MPPT_STEP 0.1

/* This is a struct
 * which holds a generic typed value
 * and can only change value
 * when mutex is unlocked
 */
template <typename T>
struct mutexVar {
  private:
    Mutex mutex;
    volatile T value = -1;
  public:
    /* volatile cannot be memcpy'd
     * so memcpy to a tmp var
     * then set the volatile
     */
    void setValue(unsigned char data[8]) {
      T tmp;
      memcpy(&tmp, data, sizeof(T));
      mutex.lock();
      value = tmp;
      mutex.unlock();
    }
    T getValue(void) {
      mutex.lock();
      return value;
      mutex.unlock();
    }
};

class BoostConverter {
  private:
    bool direction = true;
    float last_power = 0; // previous power
    float v_ref = 48; // voltage we want
    PwmOut pwm;
    AnalogIn voltageADC;
    AnalogIn currentADC;
  public:
    BoostConverter(PinName v, PinName i, PinName p);
    float getInputCurrent(void);
    float getInputVoltage(void);
    void PO(float vin, float iin);
    void PID(void);
    void setDuty(float duty);
    float getDuty(void);
};

class Mppt {
  private:
    CAN *can;
    AnalogIn batteryADC;
    void canLoop(void);
    bool notParsed(CANMessage msg);
    Thread thread;
    volatile bool running;
  public:
    mutexVar<float> maxOutputCurrent;
    mutexVar<float> maxOutputVoltage;
    mutexVar<uint8_t> mode;
    BoostConverter bc1;
    BoostConverter bc2;
    BoostConverter bc3;
    Mppt(void);
    ~Mppt(void);
    float getOutputVoltage(void);
    float getOutputCurrent(void);
    bool notInit(void);
    void IPID(void);
    void VPID(void);
};
