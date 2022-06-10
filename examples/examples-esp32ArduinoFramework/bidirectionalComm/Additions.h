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
   eCOMMAND = 0,
}eDataTypes;


FRTTTaskHandle TASK_SENDER;
FRTTTaskHandle TASK_RECEIVER;

FRTTQueueHandle QUEUE_TO_RECEIVER;
FRTTQueueHandle QUEUE_FROM_RECEIVER;

FRTTSemaphoreHandle SEMAPHORE1;
FRTTSemaphoreHandle SEMAPHORE2;

#define QUEUELENGTH 1

/* COMMANDS BETWEEN TASK_RECEIVER TO TASK_SENDER */
#define COMMAND_STOP             (0u)
#define COMMAND_SEND             (1u)
#define COMMAND_TURNLEDON        (2u)
#define COMMAND_TURNLEDOFF       (3u)


void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);