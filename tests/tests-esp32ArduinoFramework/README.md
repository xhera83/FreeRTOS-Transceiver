## 1. Unit-Tests

Ive created some unit-tests to constantly test library changes against unwanted behaviour. They're doing their job for now but need some refactoring 
plus documentation at some point. 

## 2. Setup for Unit-Tests

- Each folder has a ```src``` folder with unit-tests in it. Each folder also contains a ```platformio.ini```.
- In order to run unit tests for the library, you need to install the plaformio visual studio code extension (two options from here):
    1. Then on the left-hand side click on the platformio alien logo > QUICK ACCESS > PIO Home > Open > Open Project
        - Now you select one of the test folders containing a ```platform.ini``` file and let platformio install dependencies
        - You might want to adjust that .ini file (baudrate,board etc)
        - Click 'build' and then 'flash' on the bottom bar.
        - All tests use the newest FreeRTOS-Transceiver version (```v1.2.0```) along with AUnit library https://github.com/bxparks/AUnit/tree/v1.6.1
        - Opening each folder as a project will make platformio download the dependencies multiple times. Workaround with option ii) 
       
    2. Follow 3) and just copy all unit-test source files into the source folder of the project (you must use v1.2.0 of the library because Ive
      done some unit testing for the newer task notification feature). You additionally need to add ```lib_deps=bxparks/AUnit@^1.6.1```.
    
## 3. General installation of the library
- Install the PlatformIO visual studio code extension
    - On the left-hand side click on the platformio alien logo > QUICK ACCESS > PIO Home > Open > New Project
    - Create a new project
        - Name of the project
        - ESP32 board in use
        - Select Arduino Framework 
        - Then follow the section "Installation" here: https://registry.platformio.org/libraries/xhera83/FreeRTOS-TransceiverV1/installation
            - You basically need to add ```lib_deps=xhera83/FreeRTOS-TransceiverV1@^{VERSION}``` to the ```platformio.ini```
    - Create your project source code, then press 'build' and 'flash' on the bottom bar


