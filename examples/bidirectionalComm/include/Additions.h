/*!
 * \file        Additions.h
 * \brief       Additional data for the examples         
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include <FRTTransceiver.h>

/* datatypes recognized throughout the example */
typedef enum
{
   eCOMMAND = 0,
}eDataTypes;


FRTTransceiver_TaskHandle TASK_SENDER;
FRTTransceiver_TaskHandle TASK_RECEIVER;

FRTTransceiver_QueueHandle QUEUE_TO_RECEIVER;
FRTTransceiver_QueueHandle QUEUE_FROM_RECEIVER;

FRTTransceiver_SemaphoreHandle SEMAPHORE1;
FRTTransceiver_SemaphoreHandle SEMAPHORE2;

#define QUEUELENGTH 1

/* COMMANDS BETWEEN TASK_RECEIVER TO TASK_SENDER */
#define COMMAND_STOP             (0u)
#define COMMAND_SEND             (1u)
#define COMMAND_TURNLEDON        (2u)
#define COMMAND_TURNLEDOFF       (3u)