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

FRTTransceiver_TaskHandle TASK_ECHO;

FRTTransceiver_QueueHandle ECHO_QUEUE;

FRTTransceiver_SemaphoreHandle SEMAPHORE1; /* In this example just neccessary because the library checks if semaphores available */


#define QUEUELENGTH  (1u)

void dataDestroyer(FRTTransceiver_TempDataContainer & internalBuffer);
void dataAllocator (const FRTTransceiver_DataContainerOnQueue & origingalContainer_onQueue ,FRTTransceiver_TempDataContainer & internalBuffer);