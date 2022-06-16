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
   eCPPSTR,
}eDataTypes;


extern FRTTTaskHandle TASK_SENDER;
extern FRTTTaskHandle TASK_RECEIVER;

extern FRTTQueueHandle QUEUE_TO_RECEIVER;

extern FRTTSemaphoreHandle SEMAPHORE;

#define QUEUELENGTH 3

void dataDestroyer(FRTTTempDataContainer & internalBuffer);
void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer);
void SENDER(void *);
void RECEIVER(void *);


#endif