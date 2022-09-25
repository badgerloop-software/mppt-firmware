## MPPT Firmware

- uses FastPWM library
- add project via Mbed Studio

enable floating point and 64 bit printf in `mbed-os/platform/mbed_lib.json`:
```json
...
"minimal-printf-enable-64-bit": {
    "help": "Enable printing 64 bit integers when using minimal printf library",
    "value": true
},
"minimal-printf-enable-floating-point": {
    "help": "Enable floating point printing when using minimal printf library",
    "value": true
},
"minimal-printf-set-floating-point-max-decimals": {
    "help": "Maximum number of decimals to be printed when using minimal printf library",
    "value": 6
}
...
```
