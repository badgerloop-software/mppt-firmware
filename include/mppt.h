#include "mbed.h"

#define MPPT_TX PA_12
#define MPPT_RX PA_11

#define MPPT_BASE_ID 0x0000
#define MPPT_MODE_ID MPPT_BASE_ID + 8 // mode
#define MPPT_MOV_ID MPPT_BASE_ID + 10 // max output voltage
#define MPPT_MIC_ID MPPT_BASE_ID + 11 // max input current


class Mppt {
  private:
    struct mFloat {
      Mutex mutex;
      float value;
    } maxOutputVoltage, maxInputCurrent;

    struct mU8 {
      Mutex mutex;
      uint8_t value;
    } mode;
  public:
    bool notParsed(CANMessage msg);
};
