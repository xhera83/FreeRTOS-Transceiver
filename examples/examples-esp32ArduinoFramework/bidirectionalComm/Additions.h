/*!
 * \file        Additions.h
 * \brief       Additional data for the examples         
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#ifndef ADDITIONS_H
#define ADDITIONS_H

#include "FRTTransceiver.h"

using namespace FRTT;

#define DYNAMIC_STRUCTARRAY 0

/* datatypes recognized throughout the example */
typedef enum
{
   eCOMMAND = 0,
}eDataTypes;


extern FRTTTaskHandle TASK_SENDER;
extern FRTTTaskHandle TASK_RECEIVER;

extern FRTTQueueHandle QUEUE_TO_RECEIVER;
extern FRTTQueueHandle QUEUE_FROM_RECEIVER;

extern FRTTSemaphoreHandle SEMAPHORE1;
extern FRTTSemaphoreHandle SEMAPHORE2;

#define QUEUELENGTH 1

/* COMMANDS BETWEEN TASK_RECEIVER TO TASK_SENDER */
#define COMMAND_STOP             (0u)
#define COMMAND_SEND             (1u)
#define COMMAND_TURNLEDON        (2u)
#define COMMAND_TURNLEDOFF       (3u)


void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);
void SENDER(void *);
void RECEIVER(void *);


#endif