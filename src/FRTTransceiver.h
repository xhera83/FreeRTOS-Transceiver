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


      bool _checkValidQueueLength(uint8_t u8QueueLength);
      int _checkWaitTime(int timeMS);
      void _rearrangeTempContainerArray(uint8_t u8CommStructPos,uint8_t u8PosRemoved);
      int _getCommStruct(FRTTransceiver_TaskHandle partner);
      bool _checkForMessages(FRTTransceiver_QueueHandle queue);
      int _getAmountOfMessages(FRTTransceiver_QueueHandle queue);
      bool _hasDataInterpreters();
      bool _hasSemaphore(FRTTransceiver_TaskHandle partner, bool txLine);

   public:
      FRTTransceiver(uint8_t u8MaxPartners = 2);
      ~FRTTransceiver();
      bool addCommPartner(FRTTransceiver_TaskHandle partnersAddress = NULL,FRTTransceiver_SemaphoreHandle semaphoreRx = NULL,FRTTransceiver_SemaphoreHandle semaphoreTx = NULL,
         FRTTransceiver_QueueHandle queueRX = NULL,uint8_t u8QueueLengthRx = -1,FRTTransceiver_QueueHandle queueTX = NULL,uint8_t u8QueueLengthTx = -1,const string partnersName = string());
      bool addCommQueue(FRTTransceiver_TaskHandle partner, FRTTransceiver_QueueHandle queueRxOrTx,uint8_t u8QueueLength = -1,bool TX = false);

      #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
         bool writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8Datatype,void * data,int blockTimeWrite = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100,uint32_t u32AdditionalInfo = 0);
      #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
         bool writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8Datatype,void * data,int blockTimeWrite = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100,uint64_t u64AdditionalData = 0);
      #endif

      #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
         bool databroadcast(uint8_t u8Datatype,void * data,int blockTimeWrite = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100,uint32_t u32AdditionalInfo = 0);
      #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
         bool databroadcast(uint8_t u8Datatype,void * data,int blockTimeWrite = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100,uint64_t u64AdditionalData = 0);
      #endif
      
      
      bool readFromQueue(FRTTransceiver_TaskHandle source,int blockTime = FRTTRANSCEIVER_WAITMAX,int blockTimeTakeSemaphore = 100);
      
      bool manualDeleteAllocatedDatabufferForLine(FRTTransceiver_TaskHandle partner,uint8_t u8PositionInBuffer);
      bool manualDeleteAllAllocatedDatabuffersForLine(FRTTransceiver_TaskHandle partner);
      
      int messagesOnQueue(FRTTransceiver_TaskHandle partner);
      bool hasDataFrom(FRTTransceiver_TaskHandle partner);
      int amountOfDataInAllBuffers();

      const TempDataContainer * getNewestBufferedDataFrom(FRTTransceiver_TaskHandle partner);
      const TempDataContainer * getOldestBufferedDataFrom(FRTTransceiver_TaskHandle partner);
      const TempDataContainer * getBufferedDataFrom(FRTTransceiver_TaskHandle partner, uint8_t u8PositionInBuffer);

      void addDataAllocateCallback(void(*fp)(const DataContainerOnQueue &,TempDataContainer &));
      void addDataFreeCallback(void (*fp)(TempDataContainer &));
      string getPartnersName(FRTTransceiver_TaskHandle partner);
};

#endif