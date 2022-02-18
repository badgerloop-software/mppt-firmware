#include "boost.h"
#include "mbed.h"
#define MPPT_BASE_ID 0x0000
#define MPPT_MOC_ID MPPT_BASE_ID + 11
template <typename T> struct mutexVar {
private:
  Mutex _mutex;
  volatile T _value;

public:
  void setValue(unsigned char data[8]) {
    T tmp;
    memcpy(&tmp, data, sizeof(T));
    _mutex.lock();
    _value = tmp;
    _mutex.unlock();
  }
  T getValue(void) {
    T tmp;
    _mutex.lock();
    tmp = _value;
    _mutex.unlock();
    return tmp;
  }
};

class Mppt {
private:
  volatile bool _running;
  void canLoop(void);
  bool notParsed(CANMessage msg);
  CAN *_can;
  AnalogIn _batteryADC;
  Thread _thread;

public:
  mutexVar<float> maxOutputCurrent;
  BoostConverter bc1;
  BoostConverter bc2;
  BoostConverter bc3;
  float getOutputVoltage(void);
  float getOutputCurrent(void);
  bool notInit(void);
  Mppt(void);
  ~Mppt(void);
};
