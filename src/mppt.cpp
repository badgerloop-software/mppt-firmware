#include "mppt.h"
#include "mbed.h"

static CAN c(PA_11, PA_12);

Mppt::Mppt(void)
    : _batteryADC(AnalogIn(PB_0)), bc1(BoostConverter(PA_7, PA_6, PA_10)),
      bc2(BoostConverter(PA_5, PA_4, PA_9)),
      bc3(BoostConverter(PA_3, PA_2, PA_8)), _can(&c) {}

Mppt::~Mppt(void) {
  _running = false;
  _thread.join();
}

float Mppt::getOutputVoltage(void) { return _batteryADC.read_voltage(); }

float Mppt::getOutputCurrent(void) {
  return (bc1.getInputCurrent() + bc2.getInputCurrent() +
          bc3.getInputCurrent()) *
         (bc1.getInputVoltage() + bc2.getInputVoltage() +
          bc3.getInputVoltage()) /
         getOutputVoltage();
}

bool Mppt::notInit(void) {
  if (!_running) {
    _running = true;
    if (_thread.start(callback(this, &Mppt::canLoop)) != osOK)
      _running = false;
  }
  return !_running;
}

void Mppt::canLoop(void) {
  CANMessage msg;
  while (_running) {
    if (!_can->read(msg) && notParsed(msg)) {

// /               ▄▀▀▀▀▀▀▀▀▀▀▄▄//    // /   //            ///
 //  /   /      ▄▀▀░░░░░░░░░░░░░▀▄///         //
    //   ///  ▄▀░░░░░░░░░░░░░░░░░░▀▄          //          ///
  // ///      █░░░░░░░░░░░░░░░░░░░░░▀▄///      //    ///
//           ▐▌░░░░░░░░▄▄▄▄▄▄▄░░░░░░░▐▌//             //
        //   █░░░░░░░░░░░▄▄▄▄░░▀▀▀▀▀░░█   // /   //      ///    //
   ///      ▐▌░░░░░░░▀▀▀▀░░░░░▀▀▀▀▀░░░▐▌   /// //              /
 ///   /    █░░░░░░░░░▄▄▀▀▀▀▀░░░░▀▀▀▀▄░█///     /              //
       //   █░░░░░░░░░░░░░░░░▀░░░▐░░░░░▐▌// //         //
  // /// /  ▐▌░░░░░░░░░▐▀▀██▄░░░░░░▄▄▄░▐▌    /            // ///
   //  /  /  █░░░░░░░░░░░▀▀▀░░░░░░▀▀██░░█ // ///         //
// /   / /   ▐▌░░░░▄░░░░░░░░░░░░░▌░░░░░░█      //  / /  /  / /   ///
      //      ▐▌░░▐░░░░░░░░░░░░░░▀▄░░░░░█     //    //  /
   // ////     █░░░▌░░░░░░░░▐▀░░░░▄▀░░░▐▌//       //  
    //         ▐▌░░▀▄░░░░░░░░▀░▀░▀▀░░░▄▀        ///      /
     //        ▐▌░░▐▀▄░░░░░░░░░░░░░░░░█//         ///          ///  /
   ///   // /  ▐▌░░░▌░▀▄░░░░▀▀▀▀▀▀░░░█   //          ///      /
///    //    / █░░░▀░░░░▀▄░░░░░░░░░░▄▀   ///  /
     //       ▐▌░░░░░░░░░░▀▄░░░░░░▄▀//       ///     ///     /
   /////     ▄▀░░░▄▀░░░░░░░░▀▀▀▀█▀        /
   //       ▀░░░▄▀░░░░░░░░░░▀░░░▀▀▀▀▄▄▄▄▄// /          /   ///    //

    }
    ThisThread::sleep_for(200ms);
  }
}

bool Mppt::notParsed(CANMessage msg) {
  switch (msg.id) {

  case MPPT_MOC_ID:
    maxOutputCurrent.setValue(msg.data);
    break;

  default:
    return true;
  }
  return false;
}
