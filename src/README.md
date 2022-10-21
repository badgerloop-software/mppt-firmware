## MPPT Algorithm Source

[MPPT Algorithm Reference](https://ww1.microchip.com/downloads/en/appnotes/00001521a.pdf) Used PERTURB AND OBSERVE MPPT IMPLEMENTATION (pg. 12) and THE MAIN PROGRAM LOOP (pg. 10)
[PID Reference](https://gist.github.com/bradley219/5373998)

`P&0`: Perturb and Observe

# Table of Contents
1. [Change P, I, D terms](#pid-terms)
2. [Change Timing](#timing)
3. [Change Constants](#constants)
4. [Debug Print](#debug-print)
5. [Simulation Mode](#simulation-mode)
6. [File Descriptions](#file-descriptions)

## PID Terms

The value for floats P, I, and D can be changed in [boost.h](boost.h)

For now, Derivative is disabled regardless of `DTERM`s value

## Timing

[CYCLE_MS](boost.h): Minimum time between loop cycles. The loop will sleep until this many milliseconds has passed

[PO_DELAY](main.cpp): Number of cycles before perturbing (changing the target input voltage)

[TRACKING_DELAY](main.cpp): Number of "chances" (consecutive cycles during which output current exceeds max output current requested over CAN) before switching to regulation mode

[SAMPLE_SIZE](main.cpp): Number of cycles or samples of input voltage for P&0 (summed together, but since power is compared and SAMPLE_SIZE doesn't dynamically change, the ratio cancels out number of samples taken)


## Constants

[PO_VOLTAGE_STEP](boost.h): Voltage added or subtracted every P&O cycle (check reference if confused)

[MAXV](main.cpp): Maximum expected voltage which is used to scale the input voltage when calculating error for PID

[MAXI](main.cpp): Maximum expected voltage which is used to scale the input current when calculating error for PID

## Debug Print

*THIS SLOWS DOWN THE MCU*

Comment and uncomment debug print defines to toggle messages at `HIDDEN DEBUG` in [boost.h](boost.h).

## Simulation Mode

The `_SIMULATION` definition in [boost.h](boost.h) is the index of the boost converter you are using for simulation

Simulation means 1 of the 3 boost converters is being tested. The consequences of this can be seen in [main.cpp](main.cpp)s `#ifdef`s

To use all 3 boost converters, comment `_SIMULATION` out. *Then, the loop expects a CAN message and won't start until the max output current is set*

## File descriptions

**boost.cpp**

PID, PO, and boost converter telemetry functions

**boost.h**

PID magic constants, cycle time, simulation index, and print debugs

**mppt.cpp**

Pin connections, Initialization, battery telemetry, and CAN loop functions*

**mppt.h**

Mutex and CAN message definitions

**main.cpp**

Main Loop from reference, debug prints, more cycle timing, and scaling constants
