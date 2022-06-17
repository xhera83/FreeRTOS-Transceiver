[![PlatformIO Registry](https://badges.registry.platformio.org/packages/xhera83/library/FreeRTOS-TransceiverV1.svg)](https://registry.platformio.org/libraries/xhera83/FreeRTOS-TransceiverV1)

# Table of Contents
1. [About the project](#aboutTheProject)
    - [Motivation](#motivation)
    - [FreeRTOS-Transceiver](#FreeRTOS-Transceiver)
        - [General overview](#generalOverview)
        - [A brief look under the hood of the library](#briefLookInto)
2. [Quickstart](#quickStart)
    - [Constructor](#constructor)
    - [Adding a communication partner](#addCommunicationPartner)
    - [Adding a Multi-Sender-Queue](#multiSenderQueue)
    - [Important data for a Task Notification setup](#taskNotificationInUse)
    - [Important data for a Queue setup](#queueFeaturesInUse)
    - [First setup](#firstSetup)
4. [Features](#features)
5. [Installation](#installation)
6. [Supported devices](#supportedDevices)
7. [License](#license)
8. [Useful Resources](#resources)

# About the project <a name="aboutTheProject"></a>
## Motivation <a name="motivation"></a>
After some time using the FreeRTOS real-time operating system along with the esp32, I noticed that it's quite laborious to only use the bare FreeRTOS functionalities for inter-task communication.

Following points particularly struck me:
- X different queues had to be created to pass X different datatypes over to another task
    - Since you have to speficy the length of the queue + size of future messages during queue-creation
- If you have multiple connections to other tasks, the number of different queues will be confusing after some time
- No obvious cut between different connections (Queues defined all over the place, Who is the reading task at the end of the queue?)


&#8594; Therefore I've decided to build the FreeRTOS inter-task communication wrapper-libray **FreeRTOS-Transceiver** as a project for my college. The library is still in an early stage and will probably be, along with some other planned freertos inter-task libraries, my focus for a while (Better usage of C++, Code refactoring etc.)
New ideas and suggestions for improvement are welcome. 

## FreeRTOS-Transceiver <a name="FreeRTOS-Transceiver"></a> [**Not usable inside an ISR yet**]
The FreeRTOS-Transceiver C++ library simplifies the use of FreeRTOS inter-task communication and encapsulates related queues (tx/rx) to one communication line.<br>

## General overview <a name="generalOverview"></a> 
![FreeRTOS-TransceiverBlockdiagram](https://github.com/xhera83/FreeRTOS-Transceiver/blob/main/documentation/diagrams/FreeRTOS-TransceiverBlockdiagram.JPG?raw=true) *Example inter-task communication setup with FreeRTOS-Transceiver*

This blockdiagram shows an example of how a possible FreeRTOS-Transceiver setup could look like. We have a normal communication between 'Task A' and 'Task_Data_1'. Normal, because the communication line is only used by two tasks. 'Task A' and 'Task_Data_1' are set up this way, so that a bidirectional communication is possible. Removing one of the queues could change it to a unidirectional one. Ultimately, it is up to the developer what kind of communication is needed.

Connection types possible with the library: 
- [x] Unidirectional
- [x] Bidirectional 
- [x] Echo
- [x] Multi-sender-queue
- [ ] Multi-reader-queue (Can be set up, but not tested and certainly not as secure as other types)


There is also a multi-sender-queue in the blockdiagram above, where 'Task_Data_1', 'Task_Data_2' and 'Task_Data_3' are the senders. 'Task_DataProcessor' is the receiver, although it is important to mention that he does not know who the senders on the multi-sender-queue are. So in case that 'Task_DataProcessor' wants to talk to the senders, it must open a seperate connection to each of them.  

## A brief look under the hood of the library <a name="briefLookInto"></a>
In the following blockdiagram you can see a very short **incomplete** description about the internal library structure. Each communication line has its seperate rx buffers of length ```FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE```, but only ```queue length``` positions in buffer will be in use (because we read max. ```queue length``` amount of data into rx buffers...new data will be inserted into the buffer if old one is released first)

![FreeRTOS-TransceiverUnderTheHood](https://github.com/xhera83/FreeRTOS-Transceiver/blob/main/documentation/diagrams/FreeRTOS-TransceiverUnderTheHood.JPG?raw=true) 

*Internal rx buffers*

In order to pass different types of data to the queue, the ```FRTT::FRTTDataContainerOnQueue``` structure was introduced:
```
/* Creating a queue with FRTT::FRTTCreateQueue(queueLength) will create a queue for queueLength 
                                                        amount of FRTTDateContainerOnQueue structures */
struct FRTTDataContainerOnQueue{
    FRTTTaskHandle senderAddress;              
    void * data;                                          
    uint8_t u8DataType;                                   
    #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
    uint32_t u32AdditionalData;                           
    #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
    uint64_t u64AdditionalData;                           
    #endif

};
```
With this method it is now possible to transmit a pointer to any data, and specify the type of data with ```u8DataType```, so that the receiving task can perform an explicit cast.
The red blocks in the blockdiagram represent the data allocator/data de-allocator callbacks a user must provide to the library. Those are essentially used to copy original data (```FRTT::FRTTDataContainerOnQueue```), read from the queue, into the corresponding rx buffer position. <br>


In the allocator callback a user can now do (Basically what ever your project requires):
- Allocate memory with malloc (not recommended) and make a copy of the data the ```void * data``` points to (switch case over ```u8DataType```)
    - You can provide the length of a buffer (e.g uint8_t * buff) through the u32AdditionalData/u64AdditionalData variable and use it inside the allocator/de-allocator       callbacks
- Just copy the ```void * data``` over and make sure that the pointer points to a valid address during access
- Future implementations will introduce some sort of memory pool class which can be used inside the callbacks to dynamically allocate memory/free memory

In the de-allocator callback a user must do:
- Free the memory previously allocated with malloc
- Future implementations will introduce some sort of memory pool class which can be used inside the callbacks to dynamically allocate memory/free memory

&#8594; Using dynamic memory allocation in the allocator callback lets you create a copy of the data received, right between ```queueRead``` and ```'moveIntoBuffer'```. Through this you can keep data as long as you want in your buffers without being dependent on the sender to keep the pointer valid. Another option is to just copy the pointer over and manually copy received data into a local variable (get buffered data with the various FRTTransceiver::getxXxBufferedDataFrom() methods). 
An additional call to ```writeToQueue()``` could act as a signal (to the sender) that data has been copied (Tasknotification feature in v1.2.0).

To further understand how the library works, please take a look at the documented source code (/documentation/html/index.html) and also the examples that are provided in this repo (/examples).

## Quickstart (ESP32 only but basically works the same for the ESP8266) <a name="quickStart"></a>

### Creating a FreeRTOS-Transceiver instance <a name="constructor"></a> 

There are two different ways to create an instance of the FRTTransceiver class.

- Statically allocated array of structures holding all partner-communication

    ``` 
        FRTT::FRTTTaskHandle OWNER = (FRTT::FRTTTaskHandle)0x1;     // Use a self build TaskHandle, or acquire the real handle!
        uint8_t u8Partners = 5;
        FRTT::FRTTCommunicationPartner partners[u8Partners]         // 5 possible connection to partner tasks
        FRTT::FRTTransceiver comm(OWNER,&partners[0],u8Partners);            
    
    ```
    The length of the array must match the ```u8Partners``` parameter!
    
- Dynamically allocated inside the library

    ``` 
        FRTT::FRTTTaskHandle OWNER = (FRTT::FRTTTaskHandle)0x1;     // Use a self build TaskHandle, or acquire the real handle!
        uint8_t u8Partners = 5;
        FRTT::FRTTransceiver comm(OWNER,u8Partners); 
    
    
    ```
    Passing a ```0``` as ```u8Partners``` will increment it to ```1``` inside the constructor!
    
### Adding a communication partner <a name="addCommunicationPartner"></a> 

Before we begin with examples it is important to understand how the library tells different communication partners apart. 
The library distinguishes communication partners with the value of their ```FRTTTaskHandle``` ```(void *)```. Unless you plan to use the Task Notification functionality, you can build your own ```FRTTTaskHandles``` like this (not recommended, always acquire the real one during task-creation): 

```
    FRTT::FRTTTaskHandle TASK1 = (FRTT::FRTTTaskHandle)0x1;
    FRTTFRTTTaskHandle TASK2 = (FRTT::FRTTTaskHandle)0x2;

```
You can add a communication partner to the list like this (Read also [Important data for a Task Notification setup](#taskNotificationInUse) and [Important data for a Queue setup](#queueFeaturesInUse)):

    
        using namespace FRTT;
        
        // comm instance for 4 partner connections
        
        FRTTTaskHandle TASK1                                // This on has been acquired through FRTTCreateTask();
        FRTTTaskHandle TASK2;                               // This on has been acquired through FRTTCreateTask();
        FRTTTaskHandle TASK3;                               // This on has been acquired through FRTTCreateTask();
        
        int QUEUELENGTH = 1;
        FRTTQueueHandle QUEUE;                             // This on has been acquired through FRTTCreateQueue();
        FRTTSemaphoreHandle S;                             // This on has been acquired through FRTTCreateSemaphore();
        
        int QUEUELENGTH = 1;
        FRTTQueueHandle QUEUE1;                             // This on has been acquired through FRTTCreateQueue();
        FRTTSemaphoreHandle S1;                             // This on has been acquired through FRTTCreateSemaphore();
        
        int QUEUELENGTH = 1;
        FRTTQueueHandle QUEUE2;                             // This on has been acquired through FRTTCreateQueue();
        FRTTSemaphoreHandle S2;                             // This on has been acquired through FRTTCreateSemaphore();
        
        /* No Task Notification (because 0x0, self-build handle), only Queue-Read operations*/
        comm.addCommPartner((FRTTTaskHandle)0x1,QUEUE,QUEUELENGTH,S,nullptr,0,nullptr,"COMM-TO-TASK0x1);
        
        /* Only usable for Task Notification */
        comm.addCommPartner(TASK1,nullptr,0,nullptr,nullptr,0,nullptr,"COMM-TO-TASK1);
        
        /* Usable for Task Notification and Queue-Read operations */
        comm.addCommPartner(TASK2,QUEUE1,QUEUELENGTH,S1,nullptr,0,nullptr,"COMM-TO-TASK2);
        
        /* Usable for Task Notification and Queue-Write operations*/
        comm.addCommPartner(TASK3,nullptr,0,nullptr,QUEUE2,QUEUELENGTH,S2,"COMM-TO-TASK3");
        
        // Another comm.addCommPartner(...) will fail because we have no connections left
        
You can acquire the ***real task handles*** while task-creation with ```FRTT::FRTTCreateTask```, by passing the address to your ```FRTTTaskHandle``` as a parameter. Those ***real task handles*** are important when you try to notify that task, because FreeRTOS uses the address ```void *``` to access the tasks control block of that task!

What you cant do (reference to [Important data for a Task Notification setup](#taskNotificationInUse) and [Important data for a Queue setup](#queueFeaturesInUse)):

- Use the same ```FRTTTaskHandle``` for two connections
- Use the same ```FRTTQueueHandle``` that was previously linked to a partner (same queue for rx & tx in ```addCommPartner()``` works -> ECHO)
- Use the same ```FRTTSemaphoreHandle``` that was previously linked to a partner (same semaphore for rx & tx in ```addCommPartner()``` works -> ECHO)
- Use invalid ```FRTTTaskHandles/FRTTQueueHandles``` and plan to use them (queue read/write, task notification)

### Adding a Multi-Sender-Queue <a name="multiSenderQueue"></a>

Adding a Multi-Sender-Queue as a connection is similiar to a normal connection. In case of a multi-sender-queue you dont use a ```FRTTTaskHandle```. Instead you just provide the ```FRTTQueueHandle```. 

The Multi-Sender-Queue added to the list of connections is then accessible via ```eMultiSenderQueue::eMULTISENDERQ0``` enumerator, the second via ```eMultiSenderQueue::eMULTISENDERQ1``` enumerator and so on. Those connections are read-only.

        using namespace FRTT;
        
        // comm instance for 4 partner connections
        
        int QUEUELENGTH = 1;
        FRTTQueueHandle QUEUE;                             // This on has been acquired through FRTTCreateQueue();
        FRTTSemaphoreHandle S;                             // This on has been acquired through FRTTCreateSemaphore();
        
        int QUEUELENGTH = 1;
        FRTTQueueHandle QUEUE1;                             // This on has been acquired through FRTTCreateQueue();
        FRTTSemaphoreHandle S1;                             // This on has been acquired through FRTTCreateSemaphore();
        
        int QUEUELENGTH = 1;
        FRTTQueueHandle QUEUE2;                             // This on has been acquired through FRTTCreateQueue();
        FRTTSemaphoreHandle S2;                             // This on has been acquired through FRTTCreateSemaphore();
        
        comm.addMultiSenderPartner(QUEUE,QUEUELENGTH,S,"MULTI1");   // accessible via eMultiSenderQueue::eMULTISENDERQ0
        
        comm.addMultiSenderPartner(QUEUE1,QUEUELENGTH,S1,"MULTI2"); // accessible via eMultiSenderQueue::eMULTISENDERQ1
        
        comm.addMultiSenderPartner(QUEUE2,QUEUELENGTH,S2,"MULTI3"); // accessible via eMultiSenderQueue::eMULTISENDERQ2
        
        ....
        
        // reads from 'QUEUE' that has been added as the first queue
        comm.readFromQueue(nullptr,eMultiSenderQueue::eMULTISENDERQ0,false,100,100);
        
        ....
        
        
Being one of the senders of a Multi-Sender-Queue is simply achievable by adding the queue and also the semaphore as a normal communication ``` comm.addCommPartner(...)```. 
One sender does not know who any of the other sender tasks are, the reader on the other hand can check the ```senderAddress``` field of the ```FRTT::FRTTDataContainerOnQueue``` structure for the source of the data package (if the sender provided his real address). 
        
#### Important data for a Task Notification setup <a name="taskNotificationInUse"></a> 

- Task Notification (Receive functionality)

    - ***Owner Address***: Can be nullptr (FreeRTOS knows who the task asking for the notification state/value will be)
    - ***Partner FRTTTaskHandle***: ```FRTTTaskHandle``` != nullptr (some val > 0x0, or valid handle) && the TaskHandle has not been added yet
    - ***FRTTQueueHandle***: Can be nullptr
    - ***Semaphore***: Can be nullptr    
    
- Task Notification (Notify functionality)
    - ***Owner Address***: Can be nullptr
    - ***Partner FRTTTaskHandle***: MUST be a valid address != nullptr, acquired through ```FRTT::FRTTCreateTask``` && ```FRTTTaskHandle``` has not been                                     added yet
    - ***FRTTQueueHandle***: Can be nullptr
    - ***Semaphore***: Can be nullptr


    
#### Important data for a Queue setup <a name="queueFeaturesInUse"></a> 

- Queue (Read functionality)
    - ***Owner Address***: Can be nullptr
    - ***Partner FRTTTaskHandle***: ```FRTTTaskHandle``` != nullptr && ```FRTTTaskHandle``` has not been added yethas not been added yet. Tx-queue can be nullptr.
    - ***Semaphore***: The rx-semaphore for the rx-queue must be a valid address != nullptr, acquired through ```FRTT::FRTTCreateSemaphore``` && rx-                            semahpore has not been added yet. Tx-semaphore can be nullptr.
    
- Queue (Write functionality)
    - ***Owner Address***: Can be nullptr (But you might add it because the address of the owner will be put into data packages as the 'source')
    - ***Partner FRTTTaskHandle***: ```FRTTTaskHandle``` != nullptr && ```FRTTTaskHandle``` has not been added yet
    - ***FRTTQueueHandle***: The tx-queue must be a valid address != nullptr, acquired through ```FRTT::FRTTCreateQueue``` && tx-queue has not been added                           yet. Rx-queue can be nullptr.
    - ***Semaphore***: Tx-semaphore for the tx-queue must be a valid address != nullptr, acquired through ```FRTT::FRTTCreateSemaphore``` && tx-                                semahpore has not been added yet. Rx-semaphore can be nullptr.


### First setup <a name="firstSetup"></a> 
Lets begin with an empty arduino sketch (**Reminder: loop() is running on core 1**)

```
void setup(){}

void loop(){}
```
The minimal useful setup is a unidirectional connection between two tasks (besides 'echo' queues). 

Lets setup the code to create two tasks and other important objects. 'Task 1' will be created with 5000 bytes stacksize and priority 8 on CORE-0. 'Task 2' will be created with 5000 bytes stacksize and priority 8 on CORE-1.

```
..... 
.....

using namespace FRTT;

FRTTTaskHandle  TASK1_HANDLE;                                                                // will hold the address of TASK1
FRTTTaskHandle  TASK2_HANDLE;                                                                // will hold the address of TASK2
FRTTQueueHandle QUEUE_TO_TASK2;                                                              // queue address
FRTTSemaphoreHandle SMPH;                                                                    // unidirectional == only one semaphore needed
#define STACKSIZE 5000u
        
void TASK1 (void * pvParams)                                                                 // basic FreeRTOS style   
{
    while(TASK1_HANDLE == nullptr || TASK2_HANDLE == nullptr) vTaskDelay(pdMS_TO_TICKS(2));  // wait until freertos created tasks, we dont want nullptr
    
    for(;;){
        // here "loop()" kind of stuff
    }
    vTaskDelete(nullptr);
}

void TASK2(void * pvParams)                                                                   // basic FreeRTOS style 
{
    while(TASK1_HANDLE == nullptr || TASK2_HANDLE == nullptr) vTaskDelay(pdMS_TO_TICKS(2));   // wait until freertos created tasks, we dont want nullptr
    
    for(;;){
        // here "loop()" kind of stuff
    }
    vTaskDelete(nullptr);
} 

// Setup() runs at the beginning, so we do setup everything in here
void setup(){

    //disableCore0WDT();                                                                // maybe needed (esp32 only)      
    //disableCore1WDT();                                                                // maybe needed (esp32 only)  
    QUEUE_TO_TASK2 = FRTTCreateQueue(2);                                                // Create queue with length 2
    SMPH = FRTTCreateSemaphore();                                                       // Create semaphore
    FRTTCreateTask(TASK1,"task-1",STACKSIZE,NULL,8,&TASK1_HANDLE,0);                    // Creates TASK1 (without core param for esp8266)
    FRTTCreateTask(TASK2,"task-2",STACKSIZE,NULL,8,&TASK2_HANDLE,1);                    // Creates TASK2 (without core param for esp8266)
}

void loop(){}                                                                           // not needed anymore
```

Now that we've created both tasks, we can now proceed to establish a connection with the FreeRTOS-Transceiver library. <br>
It is very important to add the informations regarding a communication line in the right way.

Lets ***zoom*** into both tasks and setup the communication lines: 

```
......

void TASK1 (void * pvParams)                                                                    // basic FreeRTOS style   
{
    while(TASK1_HANDLE == nullptr || TASK2_HANDLE == nullptr) vTaskDelay(pdMS_TO_TICKS(2));     // wait until freertos created tasks
    
    FRTTCommunicationPartner partners[1];
    FRTTransceiver comm(TASK1_HANDLE,&partners[0],1);                                           // Add owner address and amount of desired connections 
    
    comm.addDataAllocateCallback(dataAllocator);                                                // Please check examples to understand how to create a basic callback               
    comm.addDataFreeCallback(dataDestroyer);                                                    // Please check examples to understand how to create a basic callback
    
    comm.addCommPartner(TASK2_HANDLE,nullptr,0,nullptr,QUEUE_TO_TASK2,2,SMPH,"COMM-TO-TASK2");  // Here TASK1 is the sender so add the queue and semaphore as TX
    // from here on write etc...
    for(;;){
        // here "loop()" kind of stuff                                                         
    }
    vTaskDelete(nullptr);
}

void TASK2(void * pvParams)                                                                     // basic FreeRTOS style 
{
    while(TASK1_HANDLE == nullptr || TASK2_HANDLE == nullptr) vTaskDelay(pdMS_TO_TICKS(2));     // wait until freertos created tasks
    FRTTCommunicationPartner partners[1];
    FRTTransceiver comm(TASK1_HANDLE,&partners[0],1);                                           // Add owner address and amount of desired connections 
    
    comm.addDataAllocateCallback(dataAllocator);                                                // Please check examples to understand how to create a callback         
    comm.addDataFreeCallback(dataDestroyer);                                                    // Please check examples to understand how to create a callback
    
    comm.addCommPartner(TASK2_HANDLE,QUEUE_TO_TASK2,2,SMPH,nullptr,0,nullptr,"COMM-TO-TASK1"); // Here TASK2 is the receiver so add the queue and semaphore as RX
    // from here on read etc...
    for(;;){
        // here "loop()" kind of stuff
    }
    vTaskDelete(nullptr);
} 


.....
```

Now with this setup you can proceed to write to the receiver (TASK2) and read from the sender (TASK1). Now please check out the several different examples to understand how to write/read from/to the queue (and many other things). 

## Features <a name = "features"></a>

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
  - Info, whether neccessary callbacks are supplied by the user or not
  - Amount of databroadcasts carried out
  - Amount of notifications sent
  - Amount of notifactions received
  - Latest notification value
  - FRTTransceiver object runtime in either seconds or minutes (if minutes at least 1)
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
  - Tasks can add their taskhandle as a "source" address
  - 1...n transmitter of data
  - Multi-Sender-Queue connections are read only. It is not possible to add a tx queue to this communication line.

- Queue/Buffer manipulation  
  - Check if datatype x available in buffer
  - Removing an element in buffer  
    - Removing the x. element from buffer
    - Removing the oldest element from buffer
    - Removing the newest element from buffer
    - Buffer flush
  - Queue Flush

- Task notification functionality  
  - Basic Notify/Receive Notification functionality added
    - Notifier can simply 'increment' the receving tasks notification value
    - Receiver can check for a notification value > 0 (with additional block-time in ms)
  - Extended Notify/Receive Notification functionality added
    - Multiple ways to notify the partner task (bit-mask to set in receivers notif. value, increment, only setting notif. state to pending etc.)
    - Receiver can clear his notifcation value onEntry/onExit with a u32 bit mask (with additional block-time in ms)
  - Internal 32 bit variable holding the last notification value
    - Accessible via get-method
  - Internal boolean variable holding the information whether the last call to 'NotifyReceivexXx' resulted in a notification or not
    - Accessible via get-method

- Secure access to data
  - Maximum of 2 semaphores per connection. One for the tx queue, one for the rx queue
    - E.g preventing sender from overwriting previously sent data, while the receiver does a copy (callbacks) right in that moment 
    - Enough time to dynamically allocate memory to make a copy for your internal buffers
    

## Installation <a name="installation"></a>

This library has been developed and tested on an **ESP32-WROOM-32** microcontroller inside a PlatformIO environment.
 - Example code plus an installation guide is located in [examples-esp32](/examples/examples-esp32ArduinoFramework)

FRTTransceiver library also works for the **ESP8266**
- Besides developing and testing on the ES32, Ive also tried to adjust parts for the ESP8266. An example along with an installation guide can be found in [examples-esp8266](/examples/examples-esp8266-rtos-sdk)            
    
## Supported Devices <a name= "supportedDevices"></a>

- Mainly in development for the ESP32 microcontroller

- Works for the ESP8266 but the installation is 'harder'
    - See [Installation](#installation)
    - FRTTransceiver library is not useable with the PlatformIO ```arduino-framework``` for the ESP8266 (The ```arduino-framework``` does not use FreeRTOS,instead they use the ```ESP-NONOS-SDK```)
    - FRTTransceiver library is currently not adjusted to the PlatformIO ```ESP8266-RTOS-SDK``` framework (did not get it to work yet, PlatformIO also uses a very outdated version of the ```ESP8266-RTOS-SDK```)

## License <a name="license"></a>

Apache 2.0 License

## Useful Resources <a name="resources"></a>

- https://www.freertos.org/fr-content-src/uploads/2018/07/161204_Mastering_the_FreeRTOS_Real_Time_Kernel-A_Hands-On_Tutorial_Guide.pdf

