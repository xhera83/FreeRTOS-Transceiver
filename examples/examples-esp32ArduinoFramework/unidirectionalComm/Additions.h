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
   eMESSAGE = 0,
}eDataTypes;


FRTTransceiver_TaskHandle TASK_SENDER;
FRTTransceiver_TaskHandle TASK_RECEIVER;

FRTTransceiver_QueueHandle QUEUE_TO_RECEIVER;

FRTTransceiver_SemaphoreHandle SEMAPHORE1;


#define QUEUELENGTH  (1u)

#define STOP_COMM_AT (5u)

void dataDestroyer(FRTTransceiver_TempDataContainer & internalBuffer);
void dataAllocator (const FRTTransceiver_DataContainerOnQueue & origingalContainer_onQueue ,FRTTransceiver_TempDataContainer & internalBuffer);