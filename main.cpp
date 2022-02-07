#include "mbed.h"
#include "mppt.h"

Mppt mppt;
CAN can(MPPT_RX, MPPT_TX);

int main() {
  CANMessage msg;
  while (true) {
    if (!can.read(msg)) {
      printf("No messages on CAN bus\n");
    } else if (mppt.notParsed(msg)) {
      printf("Error parsing msg\n");
    } else {
      printf("Successfully parsed msg!\n");
    }
    ThisThread::sleep_for(200);
  }
}
