#include "mbed.h"
#include "mppt.h"

Mppt mppt;

int main() {
  if (mppt.notInit()) {
    printf("ERROR couldn't init MPPT\n");
  }
  while (true) {}
}
