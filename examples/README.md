## Examples for both ESP32 and ESP8266
> **Installation guide for both architectures in their corresponding folder**

- There are currently multiple examples for the ESP32. I will refactor all examples along the way, maybe exchange some for real use case applications, but for now they definitely deliver the basic idea of how the library should be used

- I've not created many examples for the ESP8266 because the library works exactly the same as for the ESP32, besides ```FRTT::FRTTCreateTask()``` not having the ```core``` parameter. So all ESP32 examples can basically be adjusted to work on an ESP8266.

- Please do not forget that the ESP8266 has only one core, which means that you need to make an extra effort to shedule your tasks perfectly (```Task Notification``` and ```Queues``` are perfectly usable for synchronizing tasks. Ive used ```Task Notification``` for synchronizing two tasks in the ESP8266 example. Synchronizing tasks with ```Queues``` was used in /tests/tests-esp32ArduinoFramework/TaskNotificationTest/


- All examples have a ```#define DYNAMIC_STRUCTARRAY``` (set to either 0 or 1). It essentially tells all examples whether to use dynamic memory allocation for ```FRTTCommunicationPartner```  inside the library, or to provide an array of ```FRTTCommunicationPartner``` structures from outside the library and omit the ``` new/free ```.

- In FRTTransceiverSettings.h one can uncomment ``` #define FRTTRANSCEIVER_ANALYTICS_ENABLE``` for debugging purposes. This will essentially print a formatted represenation of important informations regarding a ```FRTTransceiver``` object to the console. ```FRTTransceiver::printCommunicationsSummary()``` will be available when the macro is defined.
