/*!
 * \file        Additions.h
 * \brief       Additional data for the examples
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include "FRTTransceiver.h"
#include "WString.h"

using namespace FRTT;

/* datatypes recognized throughout the example */
typedef enum
{
   eUARTBUFFER = 0,
   eSENSORBUFFER = 2,
   eMOTORBUFFER = 3,
   eCOMMAND = 4,
}eDataTypes;

FRTTTaskHandle TASK_MASTER;
FRTTTaskHandle TASK_UART_SLAVE;
FRTTTaskHandle TASK_MOTOR_SLAVE;
FRTTTaskHandle TASK_SENSOR_SLAVE;

FRTTQueueHandle MULTISENDERQ;
FRTTQueueHandle QUEUE_TO_MOTOR;
FRTTQueueHandle QUEUE_TO_SENSOR;
FRTTQueueHandle QUEUE_TO_UART;

FRTTSemaphoreHandle SEMAPHORE_MULTIQ;
FRTTSemaphoreHandle SEMAPHORE_MOTOR;
FRTTSemaphoreHandle SEMAPHORE_UART;
FRTTSemaphoreHandle SEMAPHORE_SENSOR;

#define QUEUELENGTH_MULTISENDERQ        (2u)    // multi-sender queuelength
#define QUEUELENGTH_GENERAL             (1u)    // queuelength of all other queues

#define COMMAND_SEND                    (0u)    // send current stored values
#define COMMAND_REFRESHDATA             (1u)    // read new values from uart|motor|sensor
#define COMMAND_STOP                    (2u)    // stop your task
#define COMMAND_SLEEP                   (3u)    // send tasks to sleep

#define SLEEP_MS                        (1200u)
#define COMMANDS                        (15u)

void handleSlaveWork(FRTTransceiver * comm,int * buffer,uint8_t u8Length,FRTTTaskHandle partnertask,eDataTypes datatype);
String printBuffer(int * u8Buffer, uint8_t u8Length);
void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);