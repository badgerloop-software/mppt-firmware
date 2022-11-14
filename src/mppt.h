#include "boost.h"
#include "mbed.h"
#define MPPT_BASE_ID 0x0000
#define MPPT_MOC_ID MPPT_BASE_ID + 10

template <typename T> struct mutexVar {
private:
  Mutex mutex_;
#ifdef SIMULATION_
  volatile T value_ = 7;
#else
  volatile T value_ = -1;
#endif

public:
  void setValue(unsigned char *data) {
    T tmp;
    memcpy(&tmp, data, sizeof(T));
    mutex_.lock();
    value_ = tmp;
    mutex_.unlock();
  }
  T getValue(void) {
    T tmp;
    mutex_.lock();
    tmp = value_;
    mutex_.unlock();
    return tmp;
  }
};

class Mppt {
private:
  volatile bool running_;
  void canLoop(void);
  void parse(CANMessage msg);
  CAN *can_;
  AnalogIn batteryADC_;
  Thread thread_;

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
