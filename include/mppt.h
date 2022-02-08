#include "mbed.h"

#define MPPT_TX PA_12
#define MPPT_RX PA_11
#define MPPT_BASE_ID 0x0000
#define MPPT_MODE_ID MPPT_BASE_ID + 8 // mode
#define MPPT_MOV_ID MPPT_BASE_ID + 10 // max output voltage
#define MPPT_MIC_ID MPPT_BASE_ID + 11 // max input current

template <typename T>
struct mutexVar {
  private:
    Mutex mutex;
  public:
    T value;
    void setValue(unsigned char data[8]) {
      mutex.lock();
      memcpy(&value, data, sizeof(T));
      mutex.unlock();
    }
};

class BoostConverter {
  private:
    AnalogIn voltageADC;
    AnalogIn currentADC;
  public:
    BoostConverter(PinName v, PinName i);
    float voltageIn;
    float currentIn;
    void setVoltageIn(void);
    void setCurrentIn(void);
};

class Mppt {
  private:
    BoostConverter BC1;
    BoostConverter BC2;
    BoostConverter BC3;
    mutexVar<float> maxOutputVoltage;
    mutexVar<float> maxInputCurrent;
    mutexVar<uint8_t> mode;
  public:
    Mppt();
    bool notParsed(CANMessage msg);
};
