/*!
 * \file        Additions.h
 * \brief       Additional data for the examples
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include "FRTTransceiver.h"

using namespace FRTT;

/* datatypes recognized throughout the example */
typedef enum
{
   eUARTBUFFER = 0,
   eSENSORBUFFER = 2,
   eMOTORBUFFER = 3,
   eCOMMAND = 4,
}eDataTypes;

FRTTransceiver_TaskHandle TASK_MASTER;
FRTTransceiver_TaskHandle TASK_UART_SLAVE;
FRTTransceiver_TaskHandle TASK_MOTOR_SLAVE;
FRTTransceiver_TaskHandle TASK_SENSOR_SLAVE;

FRTTransceiver_QueueHandle MULTISENDERQ;
FRTTransceiver_QueueHandle QUEUE_TO_MOTOR;
FRTTransceiver_QueueHandle QUEUE_TO_SENSOR;
FRTTransceiver_QueueHandle QUEUE_TO_UART;

FRTTransceiver_SemaphoreHandle SEMAPHORE_MULTIQ;
FRTTransceiver_SemaphoreHandle SEMAPHORE_MOTOR;
FRTTransceiver_SemaphoreHandle SEMAPHORE_UART;
FRTTransceiver_SemaphoreHandle SEMAPHORE_SENSOR;

#define QUEUELENGTH_MULTISENDERQ        (2u)    // multi-sender queuelength
#define QUEUELENGTH_GENERAL             (1u)    // queuelength of all other queues

#define COMMAND_SEND                    (0u)    // send current stored values
#define COMMAND_REFRESHDATA             (1u)    // read new values from uart|motor|sensor
#define COMMAND_STOP                    (2u)    // stop your task
#define COMMAND_SLEEP                   (3u)    // send tasks to sleep

#define SLEEP_MS                        (1200u)
#define COMMANDS                        (15u)

void handleSlaveWork(FRTTransceiver * comm,int * buffer,uint8_t u8Length,FRTTransceiver_TaskHandle partnertask,eDataTypes datatype);
String printBuffer(int * u8Buffer, uint8_t u8Length);
void dataDestroyer(FRTTransceiver_TempDataContainer & internalBuffer);
void dataAllocator (const FRTTransceiver_DataContainerOnQueue & origingalContainer_onQueue ,FRTTransceiver_TempDataContainer & internalBuffer);