[![PlatformIO Registry](https://badges.registry.platformio.org/packages/xhera83/library/FreeRTOS-TransceiverV1.svg)](https://registry.platformio.org/libraries/xhera83/FreeRTOS-TransceiverV1)

# Table of Contents :page_with_curl:
1. [About the project](#aboutTheProject)
    - [Motivation](#motivation)
    - [FreeRTOS-Transceiver](#FreeRTOS-Transceiver)
        - [General overview](#generalOverview)
        - [A brief look under the hood of the library](#briefLookInto)
2. [Quickstart](#quickStart)
3. [Features](#features)
4. [Installation](#installation)
5. [Supported devices](#supportedDevices)
6. [License](#license)
7. [Useful Resources](#resources)

# About the project <a name="aboutTheProject"></a>
## Motivation :muscle: <a name="motivation"></a>
After some time using the FreeRTOS real-time operating system along with the esp32, I noticed that it's quite laborious to only use the bare FreeRTOS functionalities for inter-task communication.

Following points particularly struck me:
- X different queues had to be created to pass X different datatypes over to another task
    - Since you have to speficy the length of the queue + size of future messages during queue-creation
- If you have multiple connections to other tasks, the number of different queues will be confusing after some time
- No obvious cut between different connections (Queues defined all over the place, Who is the reading task at the end of the queue?)


:arrow_forward: Therefore I've decided to develop the FreeRTOS inter-task communication wrapper libray **FreeRTOS-Transceiver**.

## FreeRTOS-Transceiver <a name="FreeRTOS-Transceiver"></a> [**Not usable inside an ISR yet**]
The FreeRTOS-Transceiver C++ library simplifies the use of FreeRTOS inter-task communication and encapsulates related queues (tx/rx) to one communication line.<br>

### General overview <a name="generalOverview"></a> 
![FreeRTOS-TransceiverBlockdiagram](https://github.com/xhera83/FreeRTOS-Transceiver/blob/main/documentation/diagrams/FreeRTOS-TransceiverBlockdiagram.JPG?raw=true) *Example inter-task communication setup with FreeRTOS-Transceiver*

This blockdiagram shows an example of how a possible FreeRTOS-Transceiver setup could look like. We have a normal communication between 'Task A' and 'Task_Data_1'. Normal, because the communication line is only used by two tasks. 'Task A' and 'Task_Data_1' are set up this way, so that a bidirectional communication is possible. Leaving one of the queues out could change it to a unidirectional one. Ultimately it is up to the developer what kind of communication is needed.

Connection types possible with the library: 
- Bidirectional :white_check_mark:
- Unidirectional :white_check_mark:
- Echo (e.g 'Task A' to 'Task A') :white_check_mark:
- Multi-sender-queue (The receiving task can not add a tx queue, since multi-reader feature is not available) :white_check_mark:
- Multi-reader-queue :x:

There is also a multi-sender-queue in the blockdiagram above, where 'Task_Data_1', 'Task_Data_2' and 'Task_Data_3' are the senders. 'Task_DataProcessor' is the receiver, although it is important to mention that he does not know who the senders on the multi-sender-queue are. So in case that 'Task_DataProcessor' wants to talk to the senders, it must open a seperate connection to each of them.  
### A brief look under the hood of the library <a name="briefLookInto"></a>
In the following blockdiagram you can see a very short **incomplete** description about the internal library structure. Each communication line has its seperate rx buffers of length ```FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE```, but only the length of the queue will be used (because we read max. ```queue length``` amount of data into rx buffers)

![FreeRTOS-TransceiverUnderTheHood](https://github.com/xhera83/FreeRTOS-Transceiver/blob/main/documentation/diagrams/FreeRTOS-TransceiverUnderTheHood.JPG?raw=true) 

*Internal rx buffers*

In order to send different types of data over the queue, the ```FRTTransceiver_DataContainerOnQueue``` structure was introduced:
```
struct FRTTransceiver_DataContainerOnQueue{
    FRTTransceiver_TaskHandle senderAddress;              
    void * data;                                          
    uint8_t u8DataType;                                   
    #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
    uint32_t u32AdditionalData;                           
    #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
    uint64_t u64AdditionalData;                           
    #endif

};
```
With this method now it is possible to transmit a pointer to any data, and specify the type of data with ```u8DataType```, so that the receiving task can perform a cast.
The red blocks in the blockdiagram represent the data allocator/data de-allocator callbacks a user must provide to the library. Those are essentially used to copy original data (```FRTTransceiver_DataContainerOnQueue```), read from the queue, into the corresponding rx buffer position. <br>


In the allocator callback a user can now do (Basically what ever your project requires):
- Allocate memory with malloc (not advised) and make a copy of the data the ```void * data``` points to (switch case over ```u8DataType```)
- Just copy it over and make sure that the pointer points to a valid address during access
- Future implementations will introduce some sort of memory pool

In the de-allocator callback a user must do:
- Free the memory previously allocated with malloc
- Future implementations will introduce some sort of memory pool

To further understand how the library works, please take a look at the documented source code (/documentation/html/index.html) and also the examples that are provided in this repo (/examples).
# Quickstart <a name="quickStart"></a>
Lets begin with an empty arduino sketch (**Reminder: loop() is running on core 1**)

```
void setup(){}

void loop(){}
```
The minimal useful setup is a unidirectional connection between two tasks (besides 'echo' queues). 

Lets setup the code to create two tasks and other important objects. 'Task 1' will be created with 5000 bytes stacksize and priority 8 on CORE-0. 'Task 2' will be created with 5000 bytes stacksize and priority 8 on CORE-1.

```
FRTTransceiver_TaskHandle  TASK1_HANDLE;                                                // will hold the address of TASK1
FRTTransceiver_TaskHandle  TASK2_HANDLE;                                                // will hold the address of TASK2
FRTTransceiver_QueueHandle QUEUE_TO_TASK2;                                              // queue address
FRTTransceiver_SemaphoreHandle SMPH;                                                    // unidirectional == only one semaphore needed
#define STACKSIZE 5000u
        
void TASK1 (void *)                                                                     // basic FreeRTOS style   
{
    while(TASK1_HANDLE == NULL || TASK2_HANDLE == NULL) vTaskDelay(pdMS_TO_TICKS(2));   // wait until freertos created tasks
    
    for(;;){
        // here "loop()"
    }
    vTaskDelete(NULL);
}

void TASK2(void *)                                                                      // basic FreeRTOS style 
{
    while(TASK1_HANDLE == NULL || TASK2_HANDLE == NULL) vTaskDelay(pdMS_TO_TICKS(2));   // wait until freertos created tasks
    
    for(;;){
        // here "loop()" 
    }
    vTaskDelete(NULL);
} 

// Setup() runs at the beginning, so we do setup everything in here
void setup(){
    //disableCore0WDT();                                                                // maybe needed      
    //disableCore1WDT();                                                                // maybe needed
    QUEUE_TO_TASK2 = FRTTransceiver_CreateQueue(2);                                     // Create queue with length 2
    SMPH = FRTTransceiver_CreateSemaphore();                                            // Create semaphore
    xTaskCreatePinnedToCore(TASK1,"task-1",STACKSIZE,NULL,8,&TASK1_HANDLE,0);           // Creates TASK1
    xTaskCreatePinnedToCore(TASK2,"task-2",STACKSIZE,NULL,8,&TASK2_HANDLE,1);           // Creates TASK2
}

void loop(){}                                                                           // not needed anymore
```

Now that we've created both tasks, we can now proceed to establish a connection through the FreeRTOS-Transceiver library. <br>
It is very important to add the informations regarding a communication line in the right way.

Lets zoom into both tasks: 

```
void TASK1 (void *)                                                                     // basic FreeRTOS style   
{
    while(TASK1_HANDLE == NULL || TASK2_HANDLE == NULL) vTaskDelay(pdMS_TO_TICKS(2));   // wait until freertos created tasks
    FRTTransceiver comm(TASK1_HANDLE,1);                                                // Add owner address and amount of desired connections 
    
    comm.addDataAllocateCallback(dataAllocator);                                        // Please check examples to understand how to create a callback               
    comm.addDataFreeCallback(dataDestroyer);                                            // Please check examples to understand how to create a callback
    
    comm.addCommPartner(TASK2_HANDLE,NULL,0,NULL,QUEUE_TO_TASK2,2,SMPH,"COMM-TO-TASK2"); // Here TASK1 is the sender so add the queue and semaphore as TX
    // from here on write etc...
    for(;;){
        // here "loop()" kind of stuff                                                         
    }
    vTaskDelete(NULL);
}

void TASK2(void *)                                                                      // basic FreeRTOS style 
{
    while(TASK1_HANDLE == NULL || TASK2_HANDLE == NULL) vTaskDelay(pdMS_TO_TICKS(2));   // wait until freertos created tasks
    FRTTransceiver comm(TASK1_HANDLE,1);                                                // Add owner address and amount of desired connections 
    
    comm.addDataAllocateCallback(dataAllocator);                                        // Please check examples to understand how to create a callback               
    comm.addDataFreeCallback(dataDestroyer);                                            // Please check examples to understand how to create a callback
    
    comm.addCommPartner(TASK2_HANDLE,QUEUE_TO_TASK2,2,SMPH,NULL,0,NULL,"COMM-TO-TASK2"); // Here TASK2 is the receiver so add the queue and semaphore as RX
    // from here on read etc...
    for(;;){
        // here "loop()" kind of stuff
    }
    vTaskDelete(NULL);
} 
```

Now with this setup you can proceed to write to 'TASK2' and read from 'TASK1'. Please now check out the several different examples to understand how to write/read from the queue. 

# Features <a name = "features"></a>

- Passing data over the queue  
  - Sending data to every possible task
  - Simultaneous transmission of x different datatypes (over the same queue)
  - Simultaneous transmission of x different datatypes to y different queues (databroadcast)
  
- Receiving data over the queue  
  - Receiving data sent by any task in the system

- Formatted representation of details regarding all connections to other tasks
  - Address of the owner task
  - Maximum possible connections
  - Amount of current normal connections to partner tasks
  - Amount of current multi-sender-queue connections
  - Info whether neccessary callbacks are supplied by the user
  - Amount of databroadcasts carried out
  - Informations for each connection
    - Name of the communication partner
    - Address of the communication partner
    - Communication type (Normal, Multi-Sender-Queue, Echo)
    - Information whether tx/rx queues are ON or OFF
    - Length of tx/rx queues
    - Amount of datapackages sent
    - Amount of datapackages received
    - Information if buffered data available

- A single queue can be used by multiple tasks (Multi-Sender-Queue)
  - Tasks add their task handle as a "source"
  - 1...n transmitter of data
  - Multi-Sender-Queue connections are read only. It is not possible to add a tx queue to this connection.

- Queue/Buffer manipulation  
  - Check if datatype x available in buffer
  - Removing an element in buffer  
    - Removing the x. element from buffer
    - Removing the oldest element from buffer
    - Removing the newest element from buffer
    - Buffer flush
  - Queue Flush

- Task notification functionality  
  - TBA. Is planned

- Secure access to data
  - Maximum of 2 semaphores per connection. One for the tx queue, one for the rx queue

# Installation <a name="installation"></a>

This library has been developed and tested on an **ESP32-WROOM-32** microcontroller inside a PlatformIO environment.
- Install the PlatformIO VSCODE extension
    - Create a new project
        - Name of the project
        - ESP32 board in use
        - Select Arduino Framework 
        - Then follow the section "Installation" here: https://registry.platformio.org/libraries/xhera83/FreeRTOS-TransceiverV1/installation

    - Create your project, then build and flash

FRTTransceiver library also works for the **ESP8266**
- Projects can be set up in a ESP-IDF style: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html
    - An installation guide (linux, docker file) + ESP8266 examples in "/examples" will be provided in a feature patch
    - FRTTransceiver library must be included as a "component". Infos regarding projects folder structure here: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/api-guides/build-system.html
    - FRTTransceiver library uses C++, so all Files should be with an .cpp extension. 
    - Main.cpp's app_main() (entry point for your projects software) function must be treated as a C function. Enclose as follows:
            
```  
#ifdef __cplusplus
extern "C"{
#endif

void app_main()
{
    // Very first code of your project
}

#ifdef __cplusplus
}
#endif

```
            
    

# Supported Devices <a name= "supportedDevices"></a>

- Mainly in development for the ESP32 microcontroller

- Works for the ESP8266 but the installation is 'harder'
    - See [Installation](#installation)
    - FRTTransceiver library is not useable with the PlatformIO Arduino-Framework for the ESP8266 (The Arduino-Framework does not use FreeRTOS, they use the ESP-NONOS-SDK)
    - FRTTransceiver library is currently not adjusted to the PlatformIO ESP8266-RTOS-SDK framework (did not get it to work yet, PlatformIO also uses a very outdated version of the ESP8266-RTOS-SDK)

# License <a name="license"></a>

Apache 2.0 License

# Useful Resources <a name="resources"></a>

- https://www.freertos.org/fr-content-src/uploads/2018/07/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf