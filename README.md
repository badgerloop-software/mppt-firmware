# MPPT Firmware

### Setup

1. Download Mbed Studio

2. Clone Project

  - Open Mbed Studio and go to `File -> Import Program...`
  - Paste the url of this repo

3. Enable floating point printing

cd into the project and open `mbed-os/platform/mbed_lib.json` with your favorite editor

then, make sure to change the file so the following lines match

```json
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
```

### Usage

read [this](src/README.md)
