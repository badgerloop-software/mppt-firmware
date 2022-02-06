#include "mppt.h"
#include "mbed.h"

int Mppt::parseMsg(CANMessage msg) {
  switch (msg.id) {
    case MPPT_MOV_ID:
      memcpy(&maxOutputVoltage, msg.data, 4);
      printf("Set Max Output Voltage to %u\n",maxOutputVoltage);
      break;
    case MPPT_MIC_ID:
      memcpy(&maxInputCurrent, msg.data, 4);
      printf("Set Max Input Current to %u\n",maxInputCurrent);
      break;
    case MPPT_MODE_ID:
      memcpy(&mode, msg.data, 1);
      printf("Set Mode to %u\n",mode);
      break;
    default:
      return 1;
  }
  return 0;
}
