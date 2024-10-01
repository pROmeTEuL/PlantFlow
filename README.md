# _PlantFlow hardware_

This is the hardware code for the PlantFlow plant pot.

## Sensors:
    - Temperature & humidity sensor
    - Water level sensor
    - Soil moisture sensor
    - Brightness sensor

## Motors:
    - Water pump

## Other services:
    - Communication with the server

## How to run:
    export using esp's script:
        # . [path_to]/export.sh
    build and deploy on esp:
        # idf.py build
        # idf.py flash -p [port;ex: /dev/ttyUSB0]
    optional:
        # idf.py monitor -p [port;ex: /dev/ttyUSB0]
