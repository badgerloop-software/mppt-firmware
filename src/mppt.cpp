#include "mppt.h"
#include "mbed.h"


bool Mppt::notParsed(CANMessage msg) {
  switch (msg.id) {

    case MPPT_MOV_ID:
      maxOutputVoltage.mutex.lock();
      memcpy(&maxOutputVoltage.value, msg.data, 4);
      printf("Set Max Output Voltage to %.6f\n",maxOutputVoltage.value);
      maxOutputVoltage.mutex.unlock();
      break;

    case MPPT_MIC_ID:
      maxInputCurrent.mutex.lock();
      memcpy(&maxInputCurrent.value, msg.data, 4);
      printf("Set Max Input Current to %.6f\n",maxInputCurrent.value);
      maxInputCurrent.mutex.unlock();
      break;

    case MPPT_MODE_ID:
      mode.mutex.lock();
      memcpy(&mode.value, msg.data, 1);
      printf("Set Mode to %u\n",mode.value);
      mode.mutex.unlock();
      break;

    default:
      return true;
  }
  return false;
}
