## 1. Examples

- Unidirectional communication between two tasks (unidirectionalComm)
- Bidirectional communication between two tasks (bidirectionalComm)
- Usage of dynamic memory allocation inside the callbacks to make a copy of received data right away (dynMemoryCallbacks)
- Multiple datatypes on one queue. Other examples also pass multiple datatypes over one queue (multiDatatypeOnQueue)
- Multi-Sender-Queue (multiple senders on one queue) (oneQueueMultipleUser)
- Echo communication. Task adds same (queue + semaphore) as rx line and and tx line for one communication (simpleEcho)
- Task notification feature used to signal what data should be put on the queue (taskNotification)

## 2. Setup for examples

- Each folder has a ```src``` folder with the example code. Each folder also contains a ```platformio.ini```.
- In order to run ESP32 examples you need to install the plaformio visual studio code extension (two options from here):
    1. Then on the left-hand side click on the platformio alien logo > QUICK ACCESS > PIO Home > Open > Open Project
        - Now you select one of the example folders containing a ```platform.ini``` file and let platformio install dependencies
        - You might want to adjust that .ini file (baudrate,board etc)
        - Click 'build' and then 'flash' on the bottom bar.
        - All examples use the newest FreeRTOS-Transceiver version (```v1.2.0```)
        - Opening each folder as a project will make platformio download the dependencies multiple times. Workaround with option ii) 
    2. Follow 3) and just copy all example source files into the source folder of the project (you must use v1.2.0 of the library)
    
## 3. General installation of the library
- Install the PlatformIO visual studio code extension
    - On the left-hand side click on the platformio alien logo > QUICK ACCESS > PIO Home > Open > New Project
    - Create a new project
        - Name of the project
        - ESP32 board in use
        - Select Arduino Framework 
        - Then follow the section "Installation" here: https://registry.platformio.org/libraries/xhera83/FreeRTOS-TransceiverV1/installation
            - You basically need to add ```lib_deps``` together with ```xhera83/FreeRTOS-TransceiverV1@^{VERSION}``` to the ```platformio.ini```
    - Create your project source code, then press 'build' and 'flash' on the bottom bar

