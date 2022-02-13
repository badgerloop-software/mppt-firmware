#include "mppt.h"
#include "mbed.h"

/* static means it will last the whole program 
 * and can only be accessed in this file
 * 
 * CAN objects are not copyable 
 * meaning they cannot copied into the class,
 * like the AnalogIn and BoostConverter can,
 * they must be passed by reference 
 *
 * a static object is better than
 * using the new keyword 
 * to create it on the heap */
static CAN c(PA_11, PA_12); 

/* BoostConverter will read the ADC
 * for current in and voltage in
 */
BoostConverter::BoostConverter(PinName v, PinName i, PinName p) : voltageADC(AnalogIn(v)), currentADC(AnalogIn(i)), pwm(PwmOut(p)) {
  pwm.period_us(12); // 12uS is 83333hZ
}

/*
 * Period is already set
 * Change duty cycle
 *
 * To turn off, set duty <- 0.0
 */
void BoostConverter::setDuty(float duty) {
  pwm.write(duty);
}
float BoostConverter::getDuty(void) {
  return pwm.read();
}

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
float Mppt::getOutputVoltage(void) {
  return batteryADC.read_voltage();
}

/*
 * Vin*Iin = Vout*Iout
 * so, sum of Iin times sum of Vin
 * divided by Vout is Iout
 */
float Mppt::getOutputCurrent(void) {
  return (
    bc1.getInputCurrent()+
    bc2.getInputCurrent()+
    bc3.getInputCurrent())*
    (
    bc1.getInputVoltage()+
    bc2.getInputVoltage()+
    bc3.getInputVoltage()
    )/getOutputVoltage();
}

/*
 * vin and iin are averaged
 * over the last MPPT_AVERAGE samples
 */
void BoostConverter::PO(float vin, float iin) {
  float power = vin*iin;
  if (power<last_power) direction ^= 1; // switch dir if less power than last
  if (direction) v_ref -= MPPT_STEP;
  else v_ref += MPPT_STEP;
  last_power = power;
}

/* void constructor, because
 * we already know exactly
 * what pins the Mppt will use
 */
Mppt::Mppt(void) : batteryADC(AnalogIn(PB_0)), bc1(BoostConverter(PA_7, PA_6, PA_10)), bc2(BoostConverter (PA_5, PA_4, PA_9)), bc3(BoostConverter (PA_3, PA_2, PA_8)), can(&c) {}

/* kill the thread and join
 * before destroying the object
 */
Mppt::~Mppt(void) {
  running=false;
  thread.join();
}

/* Separate function to init
 *
 * "Eudlis taught me"
 * in Kanye Blame Game woman voice
 *
 * running is used in the loop func
 * when running true -> false, loop stops
 * and thread can be joined
 */
bool Mppt::notInit(void) {
  if (!running) {
    running = true; // so thread while loop will start running
    if (thread.start(callback(this, &Mppt::canLoop)) != osOK) // osOK == 0
      running = false; // thread didn't start, so not running
  }
  return !running; // if running, we are init, so return false
}

/* Function to pass to thread
 *
 * Checks the CAN bus
 * and parses the msg
 */
void Mppt::canLoop(void) {
  CANMessage msg;
  while (running) {
    if (!can->read(msg) && notParsed(msg)) {
      // no message or not parsed
      // && stops if no message
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

    case MPPT_MOC_ID:
      maxOutputCurrent.setValue(msg.data);
      break;

    case MPPT_MOV_ID:
      maxOutputVoltage.setValue(msg.data);
      break;

    case MPPT_MODE_ID:
      mode.setValue(msg.data);
      break;

    default:
      return true;
  }
  return false;
}
