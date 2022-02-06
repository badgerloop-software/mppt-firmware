#include "mbed.h"
#include "mppt.h"

Mppt mppt;
CAN can(MPPT_RX, MPPT_TX);

int main() {
  CANMessage msg;
  while (true) {
    if (can.read(msg) && mppt.parseMsg(msg)) {
        printf("No messages received or parsed\n");
    }
    ThisThread::sleep_for(200);
  }
  
}

