#ifndef FRTTRANSCEIVER_H
#define FRTTRANSCEIVER_H

/*!
 * \file       FRTTransceiver.h
 * \author     Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include <Arduino.h>
#include <FRTTransceiverExtension.h>
#include <string>

using namespace std;



class FRTTransceiver
{
   private:
      struct CommunicationPartner * _structCommPartners; 
      uint8_t _u8CurrCommPartners = 0;
      uint8_t _u8MaxPartners;

      funcPointer_dataAllocateCallback _dataAllocator = NULL;
      funcPointer_dataFreeCallback _dataDestroyer = NULL;

      int _checkWaitTime(int timeMS);
      int _getCommStruct(FRTTransceiver_TaskHandle partner);
      bool _checkForMessages(FRTTransceiver_QueueHandle queue);
      int _getAmountOfMessages(FRTTransceiver_QueueHandle queue);
      bool _hasDataInterpreters();
      bool _hasSemaphore(FRTTransceiver_TaskHandle partner);

   public:
      FRTTransceiver(uint8_t u8MaxPartners = 2);
      ~FRTTransceiver();
      bool addCommPartner(FRTTransceiver_TaskHandle partnersAddress = NULL,FRTTransceiver_SemaphoreHandle semaphore = NULL,FRTTransceiver_QueueHandle queueRX = NULL, \
                          FRTTransceiver_QueueHandle queueTX = NULL,const string partnersName = string());
      bool addCommQueue(FRTTransceiver_TaskHandle partner, FRTTransceiver_QueueHandle queueRxOrTx,bool TX);

      #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
         bool writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8Datatype,void * data,int blockTimeWrite = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100,uint32_t u32AdditionalInfo = 0);
      #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
         bool writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8Datatype,void * data,int blockTimeWrite = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100,uint64_t u64AdditionalData = 0);
      #endif
      
      bool readFromQueue(FRTTransceiver_TaskHandle source,int blockTime = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100);
      void manualDeleteAllocatedData(FRTTransceiver_TaskHandle partner);
      void manualDeleteAllAllocatedData();
      int messagesOnQueue(FRTTransceiver_TaskHandle partner);
      bool hasDataFrom(FRTTransceiver_TaskHandle partner);
      int waitingData();
      const TempDataContainer * getNewestDataFrom(FRTTransceiver_TaskHandle partner);
      void addDataAllocateCallback(void(*fp)(const DataContainerOnQueue &,TempDataContainer &));
      void addDataFreeCallback(void (*fp)(TempDataContainer &));
      string getPartnersName(FRTTransceiver_TaskHandle partner);
};

#endif