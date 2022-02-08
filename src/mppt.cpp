#include "mppt.h"
#include "mbed.h"

BoostConverter::BoostConverter(PinName v, PinName i) : voltageADC(AnalogIn(v)), currentADC(AnalogIn(i)) {}

void BoostConverter::setCurrentIn(void) {
  currentIn = currentADC.read_voltage();
}
void BoostConverter::setVoltageIn(void) {
  voltageIn = voltageADC.read_voltage();
}

Mppt::Mppt() : BC1(BoostConverter(PA_7, PA_6)), BC2(BoostConverter (PA_5, PA_4)), BC3(BoostConverter (PA_3, PA_2)) {}

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
