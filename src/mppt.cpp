#include "mppt.h"
#include "mbed.h"

/* static CAN bus means
 * it will last the whole program 
 * 
 * CAN objects are not copyable 
 * meaning they cannot be created
 * like the AnalogIn and BoostConverter,
 * then copied into the class,
 * they must be passed by reference 
 *
 * a static CAN object is better than
 * using the new keyword 
 * to create it on the heap */
static CAN c(PA_11, PA_12); 

/* BoostConverter will read the ADC
 * for current in and voltage in
 */
BoostConverter::BoostConverter(PinName v, PinName i) : voltageADC(AnalogIn(v)), currentADC(AnalogIn(i)) {}

/* directly read the pin rather than
 * storing the value, because
 * the value could change quickly
 */
float BoostConverter::getInputCurrent(void) {
  return currentADC.read_voltage();
}
float BoostConverter::getInputVoltage(void) {
  return voltageADC.read_voltage();
}
float Mppt::getBatteryVoltage(void) {
  return batteryADC.read_voltage();
}

/* void constructor, because
 * we already know exactly
 * what pins the Mppt will use
 */
Mppt::Mppt(void) : batteryADC(AnalogIn(PB_0)), bc1(BoostConverter(PA_7, PA_6)), bc2(BoostConverter (PA_5, PA_4)), bc3(BoostConverter (PA_3, PA_2)), can(&c) {}

/* kill the thread and join
 * before destroying the object
 */
Mppt::~Mppt(void) {
  running=false;
  thread.join();
}

/* Separate function to init
 *
 * "Eric taught me"
 * in Kanye Blame Game woman voice
 */
void Mppt::init(void) {
  running = true;
  thread.start(callback(this, &Mppt::loop));
}

/* Function to pass to thread
 *
 * Checks the CAN bus
 * and parses the msg
 */
void Mppt::loop(void) {
  CANMessage msg;
  while (running) {
    if (!can->read(msg)) {
      printf("No messages on CAN bus\n");
    } else if (notParsed(msg)) {
      printf("Error parsing msg\n");
    } else {
      printf("Successfully parsed msg!\n");
    }
    ThisThread::sleep_for(200ms);
  }
}

/* Parses msg and returns
 * true if the message was
 * not parsed.
 * Makes sense in
 * loop func logic
 */
bool Mppt::notParsed(CANMessage msg) {
  switch (msg.id) {

    case MPPT_MCC_ID:
      maxChargeCurrent.setValue(msg.data);
      break;

    case MPPT_MODE_ID:
      mode.setValue(msg.data);
      break;

    default:
      return true;
  }
  return false;
}
