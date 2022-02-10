#include "mbed.h"
#define MPPT_BASE_ID 0x0000
#define MPPT_MODE_ID MPPT_BASE_ID + 8 // mode
#define MPPT_MCC_ID MPPT_BASE_ID + 11 // max charge current

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
    /* reading a variable
     * doesn't require a
     * mutex lock and unlock
     *
     * but the value is private
     * so it can only be changed
     * via setValue, utilizing the mutex
     */
    T getValue(void) {
      return value;
    }
};

class BoostConverter {
  private:
    PwmOut pwm;
    AnalogIn voltageADC;
    AnalogIn currentADC;
  public:
    BoostConverter(PinName v, PinName i, PinName p);
    float getInputCurrent(void);
    float getInputVoltage(void);
    void setPWM(float duty);
};

class Mppt {
  private:
    CAN *can;
    AnalogIn batteryADC;
    BoostConverter bc1;
    BoostConverter bc2;
    BoostConverter bc3;
    Thread thread;
    volatile bool running;
    void loop(void);
    mutexVar<float> maxChargeCurrent;
    mutexVar<uint8_t> mode;
    bool notParsed(CANMessage msg);
    float getBatteryVoltage(void);
  public:
    Mppt(void);
    ~Mppt(void);
    bool notInit(void);
};
