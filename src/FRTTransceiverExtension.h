#ifndef FRTTRANSCEIVEREXTENSION_H
#define FRTTRANSCEIVEREXTENSION_H
/*!
 * \file       FRTTransceiverExtension.h
 * \brief      Extension of FRTTransceiver
 * \author     Xhemail Ramabaja (x.ramabaja@outlook.de)
 *
 */


#include "freertos/queue.h"
#include <FRTTransceiverSettings.h>
#include <string>

using namespace std;

#define FRTTRANSCEIVER_WAITMAX (-1)


#define FRTTRANSCEIVER_PDTRUE (pdTRUE)
#define FRTTRANSCEIVER_PDFALSE (pdFALSE)



typedef UBaseType_t FRTTransceiver_BaseType;
typedef UBaseType_t FRTTransceiver_ModeType;
typedef QueueHandle_t FRTTransceiver_QueueHandle;
typedef TaskHandle_t FRTTransceiver_TaskHandle;
typedef SemaphoreHandle_t FRTTransceiver_SemaphoreHandle;


struct DataContainerOnQueue
{
   void * data;
   uint8_t u8DataType;
   #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
   uint32_t u32AdditionalData;
   #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
   uint64_t u64AdditionalData;
   #endif
};

typedef DataContainerOnQueue TempDataContainer;

struct CommunicationPartner
{
   FRTTransceiver_TaskHandle commPartner = NULL;           
   string partnersName;   

   FRTTransceiver_SemaphoreHandle semaphoreRxQueue = NULL;
   FRTTransceiver_SemaphoreHandle semaphoreTxQueue = NULL;        

   uint8_t u8TxQueueLength = 0;
   uint8_t u8RxQueueLength = 0;

   struct DataContainerOnQueue txLineContainer[FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE];    
   struct DataContainerOnQueue rxLineContainer; 

   FRTTransceiver_QueueHandle rxQueue = NULL;              
   FRTTransceiver_QueueHandle txQueue = NULL;              

   TempDataContainer tempContainer[FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE];

   bool hasBufferedData = false;
   bool rxBufferFull = false;            
   int8_t i8CurrTempcontainerPos = -1;
};


using funcPointer_dataAllocateCallback =  void (*)(const DataContainerOnQueue &,TempDataContainer &);

using funcPointer_dataFreeCallback = void (*)(TempDataContainer &);

FRTTransceiver_QueueHandle FRTTransceiver_CreateQueue(FRTTransceiver_BaseType lengthOfQueue,FRTTransceiver_BaseType elementSize);
FRTTransceiver_SemaphoreHandle FRTTransceiver_CreateSemaphore();


#endif