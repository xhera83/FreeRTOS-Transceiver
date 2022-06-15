/*!
 * \file        Additions.h
 * \brief       Additional data for the examples
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */
#ifndef ADDITIONS_H
#define ADDITIONS_H

#include "FRTTransceiver.h"
#include "WString.h"

using namespace FRTT;

#define DYNAMIC_STRUCTARRAY 0

/* datatypes recognized throughout the example */
typedef enum
{
   eUARTBUFFER = 0,
   eSENSORBUFFER = 2,
   eMOTORBUFFER = 3,
   eCOMMAND = 4,
}eDataTypes;

extern FRTTTaskHandle TASK_MASTER;
extern FRTTTaskHandle TASK_UART_SLAVE;
extern FRTTTaskHandle TASK_MOTOR_SLAVE;
extern FRTTTaskHandle TASK_SENSOR_SLAVE;

extern FRTTQueueHandle MULTISENDERQ;
extern FRTTQueueHandle QUEUE_TO_MOTOR;
extern FRTTQueueHandle QUEUE_TO_SENSOR;
extern FRTTQueueHandle QUEUE_TO_UART;

extern FRTTSemaphoreHandle SEMAPHORE_MULTIQ;
extern FRTTSemaphoreHandle SEMAPHORE_MOTOR;
extern FRTTSemaphoreHandle SEMAPHORE_UART;
extern FRTTSemaphoreHandle SEMAPHORE_SENSOR;

#define QUEUELENGTH_MULTISENDERQ        (2u)    // multi-sender queuelength
#define QUEUELENGTH_GENERAL             (1u)    // queuelength of all other queues

#define COMMAND_SEND                    (0u)    // send current stored values
#define COMMAND_REFRESHDATA             (1u)    // read new values from uart|motor|sensor
#define COMMAND_STOP                    (2u)    // stop your task
#define COMMAND_SLEEP                   (3u)    // send tasks to sleep

#define SLEEP_MS                        (1200u)
#define COMMANDS                        (15u)

void handleSlaveWork(FRTTransceiver & comm,int * buffer,uint8_t u8Length,FRTTTaskHandle partnertask,eDataTypes datatype);
String printBuffer(int * u8Buffer, uint8_t u8Length);
void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);

void Master(void *);
void UART_SLAVE(void *);
void SENSOR_SLAVE(void *);
void MOTOR_SLAVE(void *);

#endif