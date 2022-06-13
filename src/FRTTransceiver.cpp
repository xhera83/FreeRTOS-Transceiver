/*!
 * \file       	FRTTransceiver.cpp
 * \brief      	Transceiver class methods implementation
 * \author     	Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version 	   v1.1.0
 * \copyright  	Copyright 2022 Xhemail Ramabaja
 */

#include "../include/FRTTransceiver.h"
#include <cstdio>

//#define LOG_INFO
namespace FRTT {
   #if defined(ESP32) || defined (CONFIG_IDF_TARGET_ESP32)
         void FRTTCreateTask(FRTTTaskFunction taskCode,
                           const char * const taskName,
                           const uint32_t stackbytes,
                           void * const taskParameter,
                           FRTTBaseType taskPriority,
                           FRTTTaskHandle * taskHandle,
                           const FRTTBaseType core)
         {
            if(taskCode == nullptr) return;

            if(xTaskCreatePinnedToCore(taskCode,string(taskName).size() == 0 ? "def-task-name" : taskName,stackbytes,taskParameter,taskPriority,taskHandle,core) != pdPASS)
            {
               #ifdef LOG_INFO
               printf("Task creation failed.\n");
               #endif
            }
         }
	#elif defined(ESP8266) || defined(CONFIG_IDF_TARGET_ESP8266)
         void FRTTCreateTask(FRTTTaskFunction taskCode,
                           const char * const taskName,
                           const uint16_t stackbytes,
                           void * const taskParameter,
                           FRTTBaseType taskPriority,
                           FRTTTaskHandle * taskHandle)
         {
            if(taskCode == nullptr) return;
            
            if(xTaskCreate(taskCode,string(taskName).size() == 0 ? "def-task-name" : taskName,stackbytes,taskParameter,taskPriority,taskHandle) != pdPASS)
            {
               #ifdef LOG_INFO
               printf("Task creation failed.\n");
               #endif
            }
		   }               
      
    #endif


   FRTTQueueHandle FRTTCreateQueue(FRTTBaseType lengthOfQueue)
   {

      if(lengthOfQueue <= 0 || lengthOfQueue > FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE)
      {
         #ifdef LOG_INFO
         printf("Supplied length of the queue is not valid. NULL returned [Either too small or too big]\n");
         #endif
         return nullptr;
      }

      FRTTQueueHandle queue = xQueueCreate(lengthOfQueue,sizeof(struct FRTTDataContainerOnQueue));

      if(!queue)
      {
         #ifdef LOG_INFO
         printf("Queue cannot be created [Insufficient heap memory]\n");
         #endif
      }
      else
      {
         #ifdef LOG_INFO
         printf("Queue successfully created. Queue handle returned\n");
         #endif
      }
      return queue;
   }


   FRTTSemaphoreHandle FRTTCreateSemaphore()
   {
      FRTTSemaphoreHandle semaphore = xSemaphoreCreateMutex();

      if(!semaphore)
      {
         #ifdef LOG_INFO
         printf("Semaphore cannot be created [Insufficient heap memory]\n");
         #endif
      }
      else
      {
         #ifdef LOG_INFO
         printf("Semaphore successfully created. Semaphore handle returned\n");
         #endif
      }
      return semaphore;
   }

   bool FRTTransceiver::_checkForMessages(FRTTQueueHandle txQueue)
   {
      if(txQueue)
      {
         return uxQueueMessagesWaiting(txQueue) > 0 ? true:false;
      }

      return false;
   }


   int FRTTransceiver::_getAmountOfMessages(FRTTQueueHandle queue)
   {
      if(queue)
      {
         return uxQueueMessagesWaiting(queue);
      }
      return -1;
   }

   bool FRTTransceiver::_hasDataInterpreters()
   {
      return (this->_dataAllocator && this->_dataDestroyer) ? true:false;
   }

   bool FRTTransceiver::_hasSemaphore(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar, bool bTxLine)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return false;
      }
      
      if(bUseTaskHandleVar)
      {
         if(bTxLine)
         {
            return this->_structCommPartners[pos].semaphoreTxQueue == nullptr ? false:true;
         }
         else
         {
            return this->_structCommPartners[pos].semaphoreRxQueue == nullptr ? false:true;
         }
      }
      else
      {
         return this->_structCommPartners[pos].semaphoreRxQueue == nullptr ? false:true;
      }
   }

   bool FRTTransceiver::addCommPartner(FRTTTaskHandle partner,FRTTQueueHandle queueRX,
                  uint8_t u8QueueLengthRx,FRTTSemaphoreHandle semaphoreRx,
                  FRTTQueueHandle queueTX,uint8_t u8QueueLengthTx,FRTTSemaphoreHandle semaphoreTx,const string partnersName)
   {

      if(!this->_bHasValidStruct || this->_u8CurrCommPartners + 1 > this->_u8MaxPartners)
      {
         return false;
      }

      if(partner != nullptr)
      {
         this->_structCommPartners[_u8CurrCommPartners].commPartner = partner;
      }
      else
      {
         return false;
      }

      if(queueRX != nullptr)
      {
         this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
         this->_structCommPartners[_u8CurrCommPartners].u8RxQueueLength = u8QueueLengthRx;

         if(semaphoreRx == nullptr || (this->_checkValidQueueLength(u8QueueLengthRx) == false))
         {
            return false;
         }
         this->_structCommPartners[_u8CurrCommPartners].semaphoreRxQueue = semaphoreRx;
      }

      if(queueTX != nullptr)
      {
         this->_structCommPartners[_u8CurrCommPartners].txQueue = queueTX;
         this->_structCommPartners[_u8CurrCommPartners].u8TxQueueLength = u8QueueLengthTx;

         if(semaphoreTx == nullptr || (this->_checkValidQueueLength(u8QueueLengthTx) == false))
         {
            return false;
         }
         this->_structCommPartners[_u8CurrCommPartners].semaphoreTxQueue = semaphoreTx;
      }
      
      if(partnersName.length() == 0)
      {
         this->_structCommPartners[_u8CurrCommPartners].partnersName = FRTTRANSCEIVER_DEFAULTPARTNERNAME;
      }
      else
      {
         this->_structCommPartners[_u8CurrCommPartners].partnersName = partnersName;
      }

      this->_u8CurrCommPartners++;
      return true;
   }

   bool FRTTransceiver::addMultiSenderPartner(FRTTQueueHandle queueRX,uint8_t u8QueueLengthRx,FRTTSemaphoreHandle semaphoreRx,
                                                   const string multiSenderQueueName)
   {
      if(!this->_bHasValidStruct || this->_u8CurrCommPartners + 1 > this->_u8MaxPartners)
      {
         return false;
      }

      if(queueRX != nullptr)
      {
         this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
         this->_structCommPartners[_u8CurrCommPartners].u8RxQueueLength = u8QueueLengthRx;

         if(semaphoreRx == nullptr || (this->_checkValidQueueLength(u8QueueLengthRx) == false))
         {
            return false;
         }
         this->_structCommPartners[_u8CurrCommPartners].semaphoreRxQueue = semaphoreRx;
      }
      else
      {
         return false;
      }

      if(multiSenderQueueName.length() == 0)
      {
         this->_structCommPartners[_u8CurrCommPartners].partnersName = FRTTRANSCEIVER_DEFAULTPARTNERNAMEMULTISENDERQUEUE;
      }
      else
      {
         this->_structCommPartners[_u8CurrCommPartners].partnersName = multiSenderQueueName;
      }

      this->_structCommPartners[_u8CurrCommPartners].bReadOnlyCommunication = true; /* means multisender queue, where we dont send ourselves*/
      this->_u8MultiSenderQueues++;
      this->_u8CurrCommPartners++;
      return true;
   }

   #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
   bool FRTTransceiver::writeToQueue(FRTTTaskHandle destination,uint8_t u8DataType,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint32_t u32AdditionalData)
   #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
   bool FRTTransceiver::writeToQueue(FRTTTaskHandle destination,uint8_t u8DataType,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint64_t u64AdditionalData)
   #endif
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(destination,eMultiSenderQueue::eNOMULTIQSELECTED,true);

      if(pos == -1 || !this->_hasDataInterpreters() || !this->_hasSemaphore(destination,eMultiSenderQueue::eNOMULTIQSELECTED,true,true))
      {
         #ifdef LOG_INFO
         printf("You are not allowed to write to a queue \nOne of the following things happened:\n"
               "-[no communication struct available]\n"
               "-[no callback functions for (allocating,freeing) data supplied]\n"
               "-[no semphores supplied]\n"
               "-[destination task unknown]\n");
         #endif
         return false;
      }

      if(this->_structCommPartners[pos].txQueue == nullptr || this->_checkValidQueueLength(this->_structCommPartners[pos].u8TxQueueLength) == false || data == nullptr)
      {
         #ifdef LOG_INFO
         printf("Action now allowed \nOne of the following things happened:\n"
               "-[no tx queue supplied]\n"
               "-[queue length invalid]\n"
               "-[data pointer null]\n");
         #endif
         return false;
      }


      unsigned long timeToWaitSemaphore = _checkWaitTime(blockTimeTakeSemaphore_Ms);
      unsigned long timeToWaitWrite = _checkWaitTime(blockTimeWrite_Ms);

      if(timeToWaitSemaphore == -2 || timeToWaitWrite == -2)
      {  
         return false;
      }

      timeToWaitSemaphore = (timeToWaitSemaphore == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitSemaphore));
      timeToWaitWrite = (timeToWaitWrite == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitWrite));

      SemaphoreHandle_t s = this->_structCommPartners[pos].semaphoreTxQueue;


      /* Queue Full. Manual wait */
      if(this->_structCommPartners[pos].u8TxQueueLength == this->_getAmountOfMessages(this->_structCommPartners[pos].txQueue))
      {
         /* does not end when data arrives, so if timeToWaitWrite == MAXWAIT -----> doesnt go further than below code */
         /* c++11 no aggregate initialization possible */
         struct FRTTDataContainerOnQueue tempDataContainerOnQueue;
         tempDataContainerOnQueue.senderAddress = this->_ownerAddress;
         tempDataContainerOnQueue.data = data;
         tempDataContainerOnQueue.u8DataType = u8DataType;

         #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
         tempDataContainerOnQueue.u32AdditionalData = u32AdditionalData;
         #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
         tempDataContainerOnQueue.u64AdditionalData = u64AdditionalData;
         #endif

         
         FRTTBaseType returnVal = xQueueSendToBack(this->_structCommPartners[pos].txQueue,(const void *)&tempDataContainerOnQueue,timeToWaitWrite);

         if(returnVal == pdPASS)
         {
            if(xSemaphoreTake(s,timeToWaitSemaphore) == pdFALSE)
            {
               #ifdef LOG_INFO
               printf("Semaphore was not available before block time expired.\n");
               #endif
               return false;
            }
            
            /* c++11 no aggregate initialization possible */
            this->_structCommPartners[pos].txLineContainer[this->_structCommPartners[pos].u8TxQueueLength - 1].senderAddress = this->_ownerAddress;
            this->_structCommPartners[pos].txLineContainer[this->_structCommPartners[pos].u8TxQueueLength - 1].data = data;
            this->_structCommPartners[pos].txLineContainer[this->_structCommPartners[pos].u8TxQueueLength - 1].u8DataType = u8DataType;
            #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
            this->_structCommPartners[pos].txLineContainer[this->_structCommPartners[pos].u8TxQueueLength - 1].u32AdditionalData = u32AdditionalData;
            #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
            this->_structCommPartners[pos].txLineContainer[this->_structCommPartners[pos].u8TxQueueLength - 1].u64AdditionalData = u64AdditionalData,
            #endif

            #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
            this->_structCommPartners[pos].dataPackagesSent++;
            #endif

            xSemaphoreGive(s);
            return true;
         }

         return false;
      }

      
      if(xSemaphoreTake(s,timeToWaitSemaphore) == pdFALSE)
      {
         #ifdef LOG_INFO
         printf("Semaphore was not available before block time expired.\n");
         #endif
         return false;
      }

      /* space left for another element */
      uint8_t u8MessagesOnQueue = this->_getAmountOfMessages(this->_structCommPartners[pos].txQueue);
      
      /* c++11 no aggregate initialization possible */
      this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue].senderAddress = this->_ownerAddress;
      this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue].data = data;
      this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue].u8DataType = u8DataType;
      #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
      this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue].u32AdditionalData = u32AdditionalData;
      #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
      this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue].u64AdditionalData = u64AdditionalData;
      #endif


      /* At this point we should just be able to put data on the queue without waiting. */
      FRTTBaseType returnVal = xQueueSendToBack(this->_structCommPartners[pos].txQueue,(const void *)&this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue],
                                                                                                                                                                  timeToWaitWrite);

      if(returnVal == pdPASS)
      {
         #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
         this->_structCommPartners[pos].dataPackagesSent++;
         #endif
      }
      xSemaphoreGive(s);
      return true;
      
   }


   #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
   bool FRTTransceiver::databroadcast(uint8_t u8DataType,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint32_t u32AdditionalData)
   #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
   bool FRTTransceiver::databroadcast(uint8_t u8DataType,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint64_t u64AdditionalData)
   #endif
   {  
      
      if(!this->_bHasValidStruct || this->_getAmountOfQueues(true) <= 0) return false;

      uint8_t u8SuccessCounter = 0;
      
      for(uint8_t u8I = 0; u8I < this->_u8CurrCommPartners ; u8I++)
      {
         if(this->_structCommPartners[u8I].bReadOnlyCommunication == false || this->_structCommPartners[u8I].txQueue != nullptr)
         {
            #ifdef FRTTRANSCEIVER_32BITADDITIONALDATA
            if(this->writeToQueue(this->_structCommPartners[u8I].commPartner,u8DataType,data,blockTimeWrite_Ms,blockTimeTakeSemaphore_Ms,u32AdditionalData)){
               u8SuccessCounter++;
            }
            #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
            if(this->writeToQueue(this->_structCommPartners[u8I].commPartner,u8DataType,data,blockTimeWrite_Ms,blockTimeTakeSemaphore_Ms,u64AdditionalData)){
               u8SuccessCounter++;
            }
            #endif
         }
      }
      #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
      if(u8SuccessCounter > 0)
      {
         this->_broadcastCount++;
      }
      #endif

      return (u8SuccessCounter == this->_getAmountOfQueues(true));
   }


   bool FRTTransceiver::readFromQueue(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,int blockTimeRead_Ms,int blockTimeTakeSemaphore_Ms)
   {
      if(!this->_bHasValidStruct || !this->_hasDataInterpreters() || !this->_hasSemaphore(partner,multiSenderQueue,bUseTaskHandleVar,false))
      {
         return false;
      }

      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1 || this->_structCommPartners[pos].rxQueue == nullptr)
      {
         return false;

      }

      unsigned long timeToWaitRead = this->_checkWaitTime(blockTimeRead_Ms);
      unsigned long timeToWaitSemaphore = this->_checkWaitTime(blockTimeTakeSemaphore_Ms);

      if(timeToWaitRead == -2 || timeToWaitSemaphore == -2)
      {
         return false;
      }

      timeToWaitRead = (timeToWaitRead == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitRead));
      timeToWaitSemaphore = (timeToWaitSemaphore == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitSemaphore));
      
      

      FRTTBaseType returnVal = xQueueReceive(this->_structCommPartners[pos].rxQueue,(void *)&this->_structCommPartners[pos].rxLineContainer,(TickType_t)timeToWaitRead);


      /* errQUEUE_EMPTY returned if expression true*/
      if(!(returnVal == pdPASS))
      {
         return false;
      }

      SemaphoreHandle_t s = this->_structCommPartners[pos].semaphoreRxQueue;

      if(xSemaphoreTake(s,timeToWaitSemaphore) == pdFALSE)
      {
         return false;
      }

      /* Here it needs to be checked whether we still have space in the tempcontainer array or not*/
      if(this->_structCommPartners[pos].bRxBufferFull)
      {
         /* remove oldest data */
         this->_dataDestroyer(this->_structCommPartners[pos].tempContainer[0]);
         /* rearrange array if length at least 2 */
         if(this->_structCommPartners[pos].u8RxQueueLength - 1 > 0)
         {
            this->_rearrangeTempContainerArray(pos,0);
         }
         this->_structCommPartners[pos].i8CurrTempcontainerPos--;
      }

      this->_dataAllocator(this->_structCommPartners[pos].rxLineContainer,this->_structCommPartners[pos].tempContainer[++this->_structCommPartners[pos].i8CurrTempcontainerPos]);
      this->_structCommPartners[pos].bHasBufferedData = true;

      if(this->_structCommPartners[pos].i8CurrTempcontainerPos+1 == this->_structCommPartners[pos].u8RxQueueLength)
      {
         this->_structCommPartners[pos].bRxBufferFull = true;
      }
      #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
      this->_structCommPartners[pos].dataPackagesReceived++;
      #endif

      xSemaphoreGive(s);
      return true;
   }

   bool FRTTransceiver::queueFlush(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandle,int blockTimeTakeSemaphore_Ms,bool bTxQueue)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandle);

      unsigned long timeToWaitSemaphore = this->_checkWaitTime(blockTimeTakeSemaphore_Ms);

      if(pos == -1 || timeToWaitSemaphore == -2)
      {
         return false;
      }

      if(((bTxQueue) && this->_structCommPartners[pos].txQueue == nullptr) || ((!bTxQueue) && this->_structCommPartners[pos].rxQueue == nullptr))
      {
         return false;
      }

      
      timeToWaitSemaphore = (timeToWaitSemaphore == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitSemaphore));

      xQueueReset(bTxQueue ? this->_structCommPartners[pos].txQueue : this->_structCommPartners[pos].rxQueue);

      
      return true;
   }



   bool FRTTransceiver::delOldestDatabuffForLine(FRTTTaskHandle partner)
   {
      return this->delDatabuffForLine(partner,eMultiSenderQueue::eMULTISENDERQ0,true,0);
   }
   bool FRTTransceiver::delOldestDatabuffForLine(eMultiSenderQueue multiSenderQueue)
   {
      return this->delDatabuffForLine(nullptr,multiSenderQueue,false,0);
   }

   bool FRTTransceiver::delNewestDatabuffForLine(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return false;
      }
      return this->delDatabuffForLine(partner,multiSenderQueue,bUseTaskHandleVar,this->_structCommPartners[pos].i8CurrTempcontainerPos);
   }

   bool FRTTransceiver::delDatabuffForLine(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8PositionInBuffer)
   {
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1 || !this->_hasDataInterpreters())
      {
         return false;
      }

      if(this->_structCommPartners[pos].bHasBufferedData && u8PositionInBuffer <= this->_structCommPartners[pos].i8CurrTempcontainerPos)
      {
         this->_dataDestroyer(this->_structCommPartners[pos].tempContainer[u8PositionInBuffer]);
         
         if(this->_structCommPartners[pos].i8CurrTempcontainerPos == 0)
         {
            this->_structCommPartners[pos].bHasBufferedData = false;

         }
         else
         {
            this->_rearrangeTempContainerArray(pos,u8PositionInBuffer);
         }
         this->_structCommPartners[pos].i8CurrTempcontainerPos--;
         this->_structCommPartners[pos].bRxBufferFull = false;
         return true;
      }
      return false;
   }


   bool FRTTransceiver::delAllDatabuffForLine(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {  
      /* _getCommStruct checks if a valid communication struct is available */  
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1 || !this->_hasDataInterpreters())
      {
         return false;
      }

      if(this->_structCommPartners[pos].bHasBufferedData)
      {  
         for(uint8_t u8I = 0;u8I <= this->_structCommPartners[pos].i8CurrTempcontainerPos;u8I++)
         {
            this->_dataDestroyer(this->_structCommPartners[pos].tempContainer[u8I]);
         }
         this->_structCommPartners[pos].bHasBufferedData = false;
         this->_structCommPartners[pos].bRxBufferFull = false;
         this->_structCommPartners[pos].i8CurrTempcontainerPos = -1;
         return true;
      }
      return false;
   }


   int FRTTransceiver::messagesOnQueue(FRTTTaskHandle partner,bool bCheckTxQueue)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,eMultiSenderQueue::eNOMULTIQSELECTED,true);

      if(pos == -1)
      {
         return -1;
      }

      FRTTQueueHandle temp = (bCheckTxQueue ? this->_structCommPartners[pos].txQueue:this->_structCommPartners[pos].rxQueue);
      if(temp == nullptr)
      {
         return -1;
      }

      return this->_getAmountOfMessages(temp);
   }

   int FRTTransceiver::messagesOnQueue(eMultiSenderQueue multiSenderQueue)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(nullptr,multiSenderQueue,false);

      if(pos == -1)
      {
         return -1;
      }

      if(this->_structCommPartners[pos].rxQueue == nullptr)
      {
         return -1;
      }

      return this->_getAmountOfMessages(this->_structCommPartners[pos].rxQueue);
   }

   bool FRTTransceiver::hasDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return false;
      }

      return this->_structCommPartners[pos].bHasBufferedData;
   }


   int FRTTransceiver::bufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return -1;
      }

      return (this->_structCommPartners[pos].bHasBufferedData ? this->_structCommPartners[pos].i8CurrTempcontainerPos + 1:0);
   }

   int FRTTransceiver::bufferedDataInAllBuffers()
   {
      if(!this->_bHasValidStruct) return -1;

      int amountOfDataAvail = 0;
      for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
      {
         if(this->_structCommPartners[u8I].bHasBufferedData){
            amountOfDataAvail += this->_structCommPartners[u8I].i8CurrTempcontainerPos + 1;
         }
      }
      return amountOfDataAvail;
   }


   int FRTTransceiver::_getCommStruct(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {
      if(!this->_bHasValidStruct) return -1;

      if(bUseTaskHandleVar)
      {
         if(!partner)
         {
            return -1;
         }

         for(int i = 0; i < this->_u8CurrCommPartners;i++)
         {
            if(this->_structCommPartners[i].commPartner == partner)
            {
               return i;
            }
         }
         return -1;
      }
      else
      {
         if(!(static_cast<int8_t>(multiSenderQueue) >= 0 && static_cast<int8_t>(multiSenderQueue) < this->_u8MultiSenderQueues))
         {
            return -1;
         }

         uint8_t counter = 0;

         for(uint8_t u8I = 0; u8I < this->_u8CurrCommPartners;u8I++)
         {
            if(this->_structCommPartners[u8I].bReadOnlyCommunication)
            {
               if(static_cast<int8_t>(multiSenderQueue) == counter)
               {
                  return u8I;
               }
               else
               {
                  counter++;
               }
            }
         }
         return -1;
      }
   }


   int FRTTransceiver::isDatatypeInBuffer(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8Datatype)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return -1;
      }

      int counter = 0;
      if(this->_structCommPartners[pos].bHasBufferedData)
      {
         for(uint8_t u8I = 0;u8I <= this->_structCommPartners[pos].i8CurrTempcontainerPos;u8I++)
         {
            if(this->_structCommPartners[pos].tempContainer[u8I].u8DataType == u8Datatype)
            {
               counter++;
            }
         }
      }
      
      return counter;
   }

   const FRTTTempDataContainer * FRTTransceiver::getNewestBufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return nullptr;
      }

      if(this->_structCommPartners[pos].bHasBufferedData)
      {
         return (const FRTTTempDataContainer *)&this->_structCommPartners[pos].tempContainer[this->_structCommPartners[pos].i8CurrTempcontainerPos];
      }
      return nullptr;
   }


   const FRTTTempDataContainer * FRTTransceiver::getOldestBufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return nullptr;
      }

      if(this->_structCommPartners[pos].bHasBufferedData)
      {
         return (const FRTTTempDataContainer *)&this->_structCommPartners[pos].tempContainer[0];
      }
      return nullptr;
   }

   const FRTTTempDataContainer * FRTTransceiver::getBufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar, 
                                                                                                                                    uint8_t u8PositionInBuffer)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);
      
      if(pos == -1)
      {
         return nullptr;
      }

      if(this->_structCommPartners[pos].bHasBufferedData && u8PositionInBuffer <= this->_structCommPartners[pos].i8CurrTempcontainerPos)
      {
         return (const FRTTTempDataContainer *)&this->_structCommPartners[pos].tempContainer[u8PositionInBuffer];
      }
      
      return nullptr;
   }

   void FRTTransceiver::addDataAllocateCallback(void(*fP)(const FRTTDataContainerOnQueue &,FRTTTempDataContainer &))
   {
      this->_dataAllocator = fP;
   }


   void FRTTransceiver::addDataFreeCallback(void (*fP)(FRTTTempDataContainer &))
   {
      this->_dataDestroyer = fP;
   }


   string FRTTransceiver::_getPartnersName(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
   {  
      /* _getCommStruct checks if a valid communication struct is available */
      int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

      if(pos == -1)
      {
         return string("");
      }

      return this->_structCommPartners[pos].partnersName;
   }

   int FRTTransceiver::_getAmountOfQueues(bool bTxQueue)
   {
      if(!this->_bHasValidStruct) return -1;

      int amount = 0;
      for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
      {
         if(bTxQueue)
         {
            if(!(this->_structCommPartners[u8I].txQueue == nullptr)) amount++;
         }
         else
         {
            if(!(this->_structCommPartners[u8I].rxQueue == nullptr)) amount++;
         }
      }
      return amount;
   }

   int FRTTransceiver::_checkWaitTime(int time_ms)
   {
      /* WAITMAX is defined as -1, If timeMs element of [-infinite;0[, than display error! */
      if(time_ms < FRTTRANSCEIVER_WAITMAX || (time_ms > FRTTRANSCEIVER_WAITMAX && time_ms < 0))
      {
         #ifdef LOG_INFO
         printf("Nothing sent [invalid wait time specified]\n");
         #endif
         return -2;
      }
      return time_ms;
   }

   bool FRTTransceiver::_checkValidQueueLength(uint8_t u8QueueLength)
   {
      return !(u8QueueLength <= 0 || u8QueueLength > FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE);
   }

   void FRTTransceiver::_rearrangeTempContainerArray(uint8_t u8CommStructPos,uint8_t u8PosRemoved)
   {
      for(uint8_t u8I = u8PosRemoved + 1; u8I <= this->_structCommPartners[u8CommStructPos].i8CurrTempcontainerPos;u8I++)
      {
         this->_structCommPartners[u8CommStructPos].tempContainer[u8I-1] = this->_structCommPartners[u8CommStructPos].tempContainer[u8I];
      }
   }

   #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE

   void FRTTransceiver::printCommunicationsSummary()
   {
      if(!this->_bHasValidStruct) return;

      /* General Infos */
      printf("General Infos\n\n");
      printf("\tOwner address           \t\t%p\n",this->_ownerAddress == nullptr ? FRTTRANSCEIVER_UNKNOWNADDRESS : this->_ownerAddress);
      printf("\tCommunicationpartner    \t\t(%d out of %d)\n",this->_u8CurrCommPartners,this->_u8MaxPartners);
      printf("\t\t- - - > (%d of those read only)\n",this->_u8MultiSenderQueues);
      printf("\tMax partners            \t\t%d\n",this->_u8MaxPartners);
      printf("\tTX-QUEUE CONNECTIONS	 \t\t%d\n",this->_getAmountOfQueues(true));
      printf("\tRX-QUEUES CONNECTIONS	 \t\t%d\n",this->_getAmountOfQueues(false));
      printf("\tData callbacks available\t\t%s\n",this->_hasDataInterpreters() ? "yes":"no");
      printf("\tBroadcasts made         \t\t%d\n\n",this->_broadcastCount);
      
      /* Communication partners */
      
      for(uint8_t u8I = 0; u8I < this->_u8CurrCommPartners;u8I++)
      {
         printf("Line [%d]\n",u8I+1);
         printf("\tName                    \t\t%s\n",this->_structCommPartners[u8I].partnersName.c_str());
         printf("\tAddress                 \t\t%p\n",this->_structCommPartners[u8I].commPartner == nullptr ? FRTTRANSCEIVER_UNKNOWNADDRESS : this->_structCommPartners[u8I].commPartner);

         if(!this->_structCommPartners[u8I].bReadOnlyCommunication)
         {
            printf("\tComm-Type               \t\t%s\n",this->_ownerAddress == this->_structCommPartners[u8I].commPartner ? FRTTRANSCEIVER_COMMTYPE3:FRTTRANSCEIVER_COMMTYPE1);
         }
         else
         {
            printf("\tComm-Type               \t\t%s\n",FRTTRANSCEIVER_COMMTYPE2);
         }
         printf("\tTX-LINE                 \t\t%s\n",this->_structCommPartners[u8I].txQueue == nullptr ? "OFF":"ON");
         printf("\t\tLength                %d\n",this->_structCommPartners[u8I].u8TxQueueLength);
         printf("\tRX-LINE                 \t\t%s\n",this->_structCommPartners[u8I].rxQueue == nullptr ? "OFF":"ON");
         printf("\t\tLength                %d\n",this->_structCommPartners[u8I].u8RxQueueLength);
         printf("\tPackages sent           \t\t%d\n",this->_structCommPartners[u8I].dataPackagesSent);
         printf("\tPackages received       \t\t%d\n",this->_structCommPartners[u8I].dataPackagesReceived);
         printf("\tHas buffered data       \t\t%s\n",this->_structCommPartners[u8I].bHasBufferedData ? "YES":"NO");
      }
   }


   FRTTransceiver::FRTTransceiver(FRTTTaskHandle ownerAddress, uint8_t u8MaxPartners)//: _bDelete(true), _bHasValidStruct(true)
   {  
      /* Can be null. Receivers wont know who you are then....*/
      this->_ownerAddress = ownerAddress;
      
      u8MaxPartners = (u8MaxPartners == 0 ? 1:u8MaxPartners);

      this->_u8MaxPartners = u8MaxPartners;
      this->_structCommPartners = new (std::nothrow) struct FRTTCommunicationPartner[u8MaxPartners];

      if(this->_structCommPartners != nullptr)
      {
         this->_bDelete = true;
         this->_bHasValidStruct = true;
      }
   }

   FRTTransceiver::~FRTTransceiver()
   {
      if(this->_bDelete)
      {
         delete[] this->_structCommPartners;
      }
   }
}
#endif