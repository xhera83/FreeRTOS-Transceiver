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
   eINT = 0,
}eDataTypes;

extern FRTTTaskHandle TASK_ECHO;

extern FRTTQueueHandle ECHO_QUEUE;

extern FRTTSemaphoreHandle SEMAPHORE1; /* In this example just neccessary because the library checks if semaphores available */


#define QUEUELENGTH  (1u)

void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);
void ECHO(void *);
#endif