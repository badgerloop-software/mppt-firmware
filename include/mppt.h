#include "mbed.h"
#define MPPT_BASE_ID 0x0000
#define MPPT_MODE_ID MPPT_BASE_ID + 8 // mode
#define MPPT_MCC_ID MPPT_BASE_ID + 11 // max charge current

/* This is a struct
 * which holds a generic typed value
 * and can only fetch and change value
 * when mutex is unlocked
 */
template <typename T>
struct mutexVar {
  private:
    Mutex mutex;
    T value;
  public:
    void setValue(unsigned char data[8]) {
      mutex.lock();
      memcpy(&value, data, sizeof(T));
      mutex.unlock();
    }
    /* to unlock the mutex before returning,
     * the value must be copied otherwise
     * returning the value would
     * leave the mutex locked
     */
    T getValue(unsigned char data[8]) {
      mutex.lock();
      T tmp = value;
      mutex.unlock();
      return tmp;
    }
};

class BoostConverter {
  private:
    AnalogIn voltageADC;
    AnalogIn currentADC;
  public:
    BoostConverter(PinName v, PinName i);
    float getChargeCurrent(void);
};

class Mppt {
  private:
    CAN *can;
    BoostConverter bc1;
    BoostConverter bc2;
    BoostConverter bc3;
    Thread thread;
    volatile bool running;
    void loop(void);
    mutexVar<float> maxChargeCurrent;
    mutexVar<uint8_t> mode;
    bool notParsed(CANMessage msg);
  public:
    Mppt(void);
    ~Mppt(void);
    void init(void);
};
