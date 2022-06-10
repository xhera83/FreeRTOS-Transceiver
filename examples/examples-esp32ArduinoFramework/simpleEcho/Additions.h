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
   eINT = 0,
}eDataTypes;

FRTTTaskHandle TASK_ECHO;

FRTTQueueHandle ECHO_QUEUE;

FRTTSemaphoreHandle SEMAPHORE1; /* In this example just neccessary because the library checks if semaphores available */


#define QUEUELENGTH  (1u)

void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);