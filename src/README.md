# MPPT Algorithm Source

[MPPT Algorithm Reference](https://ww1.microchip.com/downloads/en/appnotes/00001521a.pdf)
- **PERTURB AND OBSERVE MPPT IMPLEMENTATION** (pg. 12)
- **THE MAIN PROGRAM LOOP** (pg. 10)

[PID Reference](https://gist.github.com/bradley219/5373998)

`P&O`: Perturb and Observe

## Table of Contents
1. [Change P, I, D terms](#pid-terms)
2. [Change Timing](#timing)
3. [Change Constants](#constants)
4. [Simulation Mode](#simulation-mode)
5. [File Descriptions](#file-descriptions)

## PID Terms

The value for floats P, I, and D can be changed in [boost.h](boost.h)

For now, Derivative is disabled regardless of `DTERM`'s value

## Timing

[CYCLE_MS](boost.h): Minimum time between loop cycles. The loop will sleep until this many milliseconds has passed

[PO_DELAY](main.cpp): Number of cycles before perturbing (changing the target input voltage)

[TRACKING_DELAY](main.cpp): Number of "chances" (consecutive cycles during which output current exceeds max output current requested over CAN) before switching to regulation mode

[SAMPLE_SIZE](main.cpp): Number of cycles or samples of input voltage for P&O (summed together, but since power is compared and SAMPLE_SIZE doesn't dynamically change, the ratio cancels out number of samples taken)


## Constants

[PO_VOLTAGE_STEP](boost.h): Voltage added or subtracted every P&O cycle (check reference if confused)

[MAXV](main.cpp): Maximum expected voltage which is used to scale the input voltage when calculating error for voltage PID

[MAXI](main.cpp): Maximum expected current which is used to scale the input current when calculating error for current PID

[`static float vref[3] = {30, 30, 30};`](main.cpp): P&O needs an initial "guess" at to where the MPP voltage is. This starting point is where we perturb from

## Simulation Mode

The `SIMULATION_` definition in [boost.h](boost.h) is the index of the boost converter you are using for simulation

Simulation means 1 of the 3 boost converters is being tested. The consequences of this can be seen in [main.cpp](main.cpp)'s `#ifdef`s

To use all 3 boost converters, comment `SIMULATION_` out. **Then, the loop expects a CAN message and won't start until the max output current is set**

## File descriptions

- [boost.cpp](boost.cpp)

PID, PO, and boost converter telemetry functions

- [boost.h](boost.h)

PID magic constants, cycle time and simulation index

- [mppt.cpp](mppt.cpp)

Pin connections, Initialization, battery telemetry, and CAN loop functions

- [mppt.h](mppt.h)

Mutex and CAN message definitions

- [main.cpp](main.cpp)

Main Loop from reference, debug prints, more cycle timing, and scaling constants
