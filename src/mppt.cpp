#include "mppt.h"
#include "mbed.h"


bool Mppt::notParsed(CANMessage msg) {
  switch (msg.id) {

    case MPPT_MOV_ID:
      maxOutputVoltage.setValue(msg.data);
      break;

    case MPPT_MIC_ID:
      maxInputCurrent.setValue(msg.data);
      break;

    case MPPT_MODE_ID:
      mode.setValue(msg.data);
      break;

    default:
      return true;
  }
  return false;
}
