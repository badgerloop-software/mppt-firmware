#include "mbed.h"
#include "mppt.h"

Mppt mppt;

int main() {
  mppt.init();
  while (true) {}
}
