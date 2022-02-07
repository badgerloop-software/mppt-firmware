#include "mppt.h"
#include "mbed.h"


bool Mppt::notParsed(CANMessage msg) {
  switch (msg.id) {

    case MPPT_MOV_ID:
      this->mutex.lock();
      memcpy(&maxOutputVoltage, msg.data, 4);
      printf("Set Max Output Voltage to %.6f\n",maxOutputVoltage);
      break;

    case MPPT_MIC_ID:
      this->mutex.lock();
      memcpy(&maxInputCurrent, msg.data, 4);
      printf("Set Max Input Current to %.6f\n",maxInputCurrent);
      break;

    case MPPT_MODE_ID:
      this->mutex.lock();
      memcpy(&mode, msg.data, 1);
      printf("Set Mode to %u\n",mode);
      break;

    default:
      return true;
  }
  this->mutex.unlock();
  return false;
}
