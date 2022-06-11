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
   eMESSAGE = 0,
   eINT = 1,
   eSTRUCT = 2,

}eDataTypes;

struct TestDType
{
   int var1;
   int var2;
};

extern FRTTTaskHandle TASK_SENDER;
extern FRTTTaskHandle TASK_RECEIVER;

extern FRTTQueueHandle QUEUE_TO_RECEIVER;

extern FRTTSemaphoreHandle SEMAPHORE1;


#define QUEUELENGTH  (3u)


void RECEIVER(void *);
void SENDER(void *);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);
void dataDestroyer(FRTTTempDataContainer & internalBuffer);


#endif