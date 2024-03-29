#include "boost.h"
#include "mbed.h"
#define MPPT_BASE_ID 0x0000
#define MPPT_MOC_ID MPPT_BASE_ID + 10

template <typename T> struct mutexVar {
private:
  Mutex _mutex;
#ifdef _SIMULATION
  volatile T _value = 7;
#else
  volatile T _value = -1;
#endif

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
  void parse(CANMessage msg);
  CAN *_can;
  AnalogIn _batteryADC;
  Thread _thread;

public:
  mutexVar<float> maxIout;
  BoostConverter bc0;
  BoostConverter bc1;
  BoostConverter bc2;
  float getVout(void);
  float getIout(void);
  bool notInit(void);
  Mppt(void);
  ~Mppt(void);
};
