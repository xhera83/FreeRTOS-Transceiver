/*!
 * \file       FRTTransceiver.cpp
 * \brief      Transceiver class methods implementation
 * \author     Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \copyright  Copyright 2022 Xhemail Ramabaja
 */

#include "../include/FRTTransceiver.h"
#include <cstdio>

//#define LOG_INFO

FRTTransceiver_QueueHandle FRTTransceiver_CreateQueue(FRTTransceiver_BaseType lengthOfQueue)
{

   if(lengthOfQueue <= 0 || lengthOfQueue > FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE)
   {
      #ifdef LOG_INFO
      printf("Supplied length of the queue is not valid. NULL returned [Either too small or too big]\n");
      #endif
      return NULL;
   }

   FRTTransceiver_QueueHandle queue = xQueueCreate(lengthOfQueue,sizeof(struct FRTTransceiver_DataContainerOnQueue));

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


FRTTransceiver_SemaphoreHandle FRTTransceiver_CreateSemaphore()
{
   FRTTransceiver_SemaphoreHandle semaphore = xSemaphoreCreateMutex();

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

bool FRTTransceiver::_checkForMessages(FRTTransceiver_QueueHandle txQueue)
{
   if(txQueue)
   {
      return uxQueueMessagesWaiting(txQueue) > 0 ? true:false;
   }

   return false;
}


int FRTTransceiver::_getAmountOfMessages(FRTTransceiver_QueueHandle queue)
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

bool FRTTransceiver::_hasSemaphore(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar, bool txLine)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);
   if(pos == -1)
   {
      return false;
   }
   
   if(bUseTaskHandleVar)
   {
      if(txLine)
      {
         return this->_structCommPartners[pos].semaphoreTxQueue == NULL ? false:true;
      }
      else
      {
         return this->_structCommPartners[pos].semaphoreRxQueue == NULL ? false:true;
      }
   }
   else
   {
      return this->_structCommPartners[pos].semaphoreRxQueue == NULL ? false:true;
   }
}

FRTTransceiver::FRTTransceiver(FRTTransceiver_TaskHandle ownerAddress, uint8_t u8MaxPartners)
{  
   /* Can be null. Receivers wont know who you are then....*/
   this->_ownerAddress = ownerAddress;
   
   u8MaxPartners = (u8MaxPartners == 0 ? 1:u8MaxPartners);

   this->_u8MaxPartners = u8MaxPartners;
   this->_structCommPartners = new struct FRTTransceiver_CommunicationPartner[u8MaxPartners];
}

FRTTransceiver::~FRTTransceiver()
{
   delete[] this->_structCommPartners;
}


bool FRTTransceiver::addCommPartner(FRTTransceiver_TaskHandle partner,FRTTransceiver_QueueHandle queueRX,
               uint8_t u8QueueLengthRx,FRTTransceiver_SemaphoreHandle semaphoreRx,
               FRTTransceiver_QueueHandle queueTX,uint8_t u8QueueLengthTx,FRTTransceiver_SemaphoreHandle semaphoreTx,const string partnersName)
{

   if(this->_u8CurrCommPartners + 1 > this->_u8MaxPartners)
   {
      return false;
   }

   if(partner != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].commPartner = partner;
   }
   else
   {
      return false;
   }

   if(queueRX != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
      this->_structCommPartners[_u8CurrCommPartners].u8RxQueueLength = u8QueueLengthRx;

      if(semaphoreRx == NULL || (this->_checkValidQueueLength(u8QueueLengthRx) == false))
      {
         return false;
      }
      this->_structCommPartners[_u8CurrCommPartners].semaphoreRxQueue = semaphoreRx;
   }

   if(queueTX != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].txQueue = queueTX;
      this->_structCommPartners[_u8CurrCommPartners].u8TxQueueLength = u8QueueLengthTx;

      if(semaphoreTx == NULL || (this->_checkValidQueueLength(u8QueueLengthTx) == false))
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

bool FRTTransceiver::addMultiSenderReadOnlyQueue(FRTTransceiver_QueueHandle queueRX,uint8_t u8QueueLengthRx,FRTTransceiver_SemaphoreHandle semaphoreRx,
                                                const string multiSenderQueueName)
{
   if(this->_u8CurrCommPartners + 1 > this->_u8MaxPartners)
   {
      return false;
   }

   if(queueRX != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
      this->_structCommPartners[_u8CurrCommPartners].u8RxQueueLength = u8QueueLengthRx;

      if(semaphoreRx == NULL || (this->_checkValidQueueLength(u8QueueLengthRx) == false))
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
bool FRTTransceiver::writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8DataType,void * data,int blockTimeWrite,int blockTimeTakeSemaphore,uint32_t u32AdditionalData)
#elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
bool FRTTransceiver::writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8DataType,void * data,int blockTimeWrite,int blockTimeTakeSemaphore,uint64_t u64AdditionalData)
#endif
{
   int pos = this->_getCommStruct(destination,eNOMULTIQSELECTED,true);

   if(!this->_hasDataInterpreters() || !this->_hasSemaphore(destination,eNOMULTIQSELECTED,true,true) || pos == -1)
   {
      #ifdef LOG_INFO
      printf("You are not allowed to write to a queue \nOne of the following things happened:\n"
            "-[no callback functions for (allocating,freeing) data supplied]\n"
            "-[no semphores supplied]\n"
            "-[destination task unknown]\n");
      #endif
      return false;
   }

   if(this->_structCommPartners[pos].txQueue == NULL || this->_checkValidQueueLength(this->_structCommPartners[pos].u8TxQueueLength) == false || data == NULL)
   {
      #ifdef LOG_INFO
      printf("Action now allowed \nOne of the following things happened:\n"
            "-[no tx queue supplied]\n"
            "-[queue length invalid]\n"
            "-[data pointer null]\n");
      #endif
      return false;
   }


   unsigned long timeToWaitSemaphore = _checkWaitTime(blockTimeTakeSemaphore);
   unsigned long timeToWaitWrite = _checkWaitTime(blockTimeWrite);

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

      FRTTransceiver_DataContainerOnQueue tempDataContainerOnQueue = 
      {
         .senderAddress = this->_ownerAddress,
         .data = data,
         .u8DataType = u8DataType,
         #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
         .u32AdditionalData = u32AdditionalData,
         #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
         .u64AdditionalData = u64AdditionalData,
         #endif
      };
      
      FRTTransceiver_BaseType returnVal = xQueueSendToBack(this->_structCommPartners[pos].txQueue,(const void *)&tempDataContainerOnQueue,timeToWaitWrite);

      if(returnVal == pdPASS)
      {
         if(xSemaphoreTake(s,timeToWaitSemaphore) == pdFALSE)
         {
            #ifdef LOG_INFO
            printf("Semaphore was not available before block time expired.\n");
            #endif
            return false;
         }
         
         this->_structCommPartners[pos].txLineContainer[this->_structCommPartners[pos].u8TxQueueLength - 1] =
         {
            .senderAddress = this->_ownerAddress,
            .data = data,
            .u8DataType = u8DataType,
            #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
            .u32AdditionalData = u32AdditionalData,
            #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
            .u64AdditionalData = u64AdditionalData,
            #endif
         };

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

   this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue] =
   {  
      .senderAddress = this->_ownerAddress,
      .data = data,
      .u8DataType = u8DataType,
      #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
      .u32AdditionalData = u32AdditionalData,
      #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
      .u64AdditionalData = u64AdditionalData,
      #endif
   };

   /* At this point we should just be able to put data on the queue without waiting. */
   FRTTransceiver_BaseType returnVal = xQueueSendToBack(this->_structCommPartners[pos].txQueue,(const void *)&this->_structCommPartners[pos].txLineContainer[u8MessagesOnQueue],
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
bool FRTTransceiver::databroadcast(uint8_t u8DataType,void * data,int blockTimeWrite,int blockTimeTakeSemaphore,uint32_t u32AdditionalData)
#elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
bool FRTTransceiver::databroadcast(uint8_t u8DataType,void * data,int blockTimeWrite,int blockTimeTakeSemaphore,uint64_t u64AdditionalData)
#endif
{
   if(this->_getAmountOfQueues(true) == 0) return false;

   uint8_t u8SuccessCounter = 0;
   
   for(uint8_t u8I = 0; u8I < this->_u8CurrCommPartners ; u8I++)
   {
      if(this->_structCommPartners[u8I].bReadOnlyCommunication == false || this->_structCommPartners[u8I].txQueue != NULL)
      {
         #ifdef FRTTRANSCEIVER_32BITADDITIONALDATA
         if(this->writeToQueue(this->_structCommPartners[u8I].commPartner,u8DataType,data,blockTimeWrite,blockTimeTakeSemaphore,u32AdditionalData)){
            u8SuccessCounter++;
         }
         #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
         if(this->writeToQueue(this->_structCommPartners[u8I].commPartner,u8DataType,data,blockTimeWrite,blockTimeTakeSemaphore,u64AdditionalData)){
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


bool FRTTransceiver::readFromQueue(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,int blockTimeRead,int blockTimeTakeSemaphore)
{
   if(!this->_hasDataInterpreters() || !this->_hasSemaphore(partner,multiSenderQueue,bUseTaskHandleVar,false))
   {
      return false;
   }

   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1 || this->_structCommPartners[pos].rxQueue == NULL)
   {
      return false;

   }

   unsigned long timeToWaitRead = this->_checkWaitTime(blockTimeRead);
   unsigned long timeToWaitSemaphore = this->_checkWaitTime(blockTimeTakeSemaphore);

   if(timeToWaitRead == -2 || timeToWaitSemaphore == -2)
   {
      return false;
   }

   timeToWaitRead = (timeToWaitRead == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitRead));
   timeToWaitSemaphore = (timeToWaitSemaphore == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitSemaphore));
   
   

   FRTTransceiver_BaseType returnVal = xQueueReceive(this->_structCommPartners[pos].rxQueue,(void *)&this->_structCommPartners[pos].rxLineContainer,(TickType_t)timeToWaitRead);


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
   if(this->_structCommPartners[pos].rxBufferFull)
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
   this->_structCommPartners[pos].hasBufferedData = true;

   if(this->_structCommPartners[pos].i8CurrTempcontainerPos+1 == this->_structCommPartners[pos].u8RxQueueLength)
   {
      this->_structCommPartners[pos].rxBufferFull = true;
   }
   #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
   this->_structCommPartners[pos].dataPackagesReceived++;
   #endif

   xSemaphoreGive(s);
   return true;
}

bool FRTTransceiver::queueFlush(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandle,int blockTimeTakeSemaphore,bool bTxQueue)
{  
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandle);

   if(pos == -1)
   {
      return false;
   }

   unsigned long timeToWaitSemaphore = this->_checkWaitTime(blockTimeTakeSemaphore);

   if(timeToWaitSemaphore == -2)
   {
      return false;
   }

   if(((bTxQueue) && this->_structCommPartners[pos].txQueue == NULL) || ((!bTxQueue) && this->_structCommPartners[pos].rxQueue == NULL))
   {
      return false;
   }

   
   timeToWaitSemaphore = (timeToWaitSemaphore == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitSemaphore));

   xQueueReset(bTxQueue ? this->_structCommPartners[pos].txQueue : this->_structCommPartners[pos].rxQueue);

   
   return true;
}



bool FRTTransceiver::manualDeleteOldestAllocatedDatabufferForLine(FRTTransceiver_TaskHandle partner)
{
   return this->manualDeleteAllocatedDatabufferForLine(partner,(eMultiSenderQueue)0,true,0);
}
bool FRTTransceiver::manualDeleteOldestAllocatedDatabufferForLine(eMultiSenderQueue multiSenderQueue)
{
   return this->manualDeleteAllocatedDatabufferForLine(NULL,multiSenderQueue,false,0);
}

bool FRTTransceiver::manualDeleteNewestAllocatedDatabufferForLine(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return false;
   }
   return this->manualDeleteAllocatedDatabufferForLine(partner,multiSenderQueue,bUseTaskHandleVar,this->_structCommPartners[pos].i8CurrTempcontainerPos);
}

bool FRTTransceiver::manualDeleteAllocatedDatabufferForLine(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8PositionInBuffer)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);
   if(!this->_hasDataInterpreters() || pos == -1)
   {
      return false;
   }

   if(this->_structCommPartners[pos].hasBufferedData && u8PositionInBuffer <= this->_structCommPartners[pos].i8CurrTempcontainerPos)
   {
      this->_dataDestroyer(this->_structCommPartners[pos].tempContainer[u8PositionInBuffer]);
      
      if(this->_structCommPartners[pos].i8CurrTempcontainerPos == 0)
      {
         this->_structCommPartners[pos].hasBufferedData = false;

      }
      else
      {
         this->_rearrangeTempContainerArray(pos,u8PositionInBuffer);
      }
      this->_structCommPartners[pos].i8CurrTempcontainerPos--;
      this->_structCommPartners[pos].rxBufferFull = false;
      return true;
   }
   return false;
}


bool FRTTransceiver::manualDeleteAllAllocatedDatabuffersForLine(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{  
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);
   if(!this->_hasDataInterpreters() || pos == -1)
   {
      return false;
   }

   if(this->_structCommPartners[pos].hasBufferedData)
   {  
      for(uint8_t u8I = 0;u8I <= this->_structCommPartners[pos].i8CurrTempcontainerPos;u8I++)
      {
         this->_dataDestroyer(this->_structCommPartners[pos].tempContainer[u8I]);
      }
      this->_structCommPartners[pos].hasBufferedData = false;
      this->_structCommPartners[pos].rxBufferFull = false;
      this->_structCommPartners[pos].i8CurrTempcontainerPos = -1;
      return true;
   }
   return false;
}


int FRTTransceiver::messagesOnQueue(FRTTransceiver_TaskHandle partner,bool bCheckTxQueue)
{
   int pos = this->_getCommStruct(partner,eNOMULTIQSELECTED,true);

   if(pos == -1)
   {
      return -1;
   }

   FRTTransceiver_QueueHandle temp = (bCheckTxQueue ? this->_structCommPartners[pos].txQueue:this->_structCommPartners[pos].rxQueue);
   if(temp == NULL)
   {
      return -1;
   }

   return this->_getAmountOfMessages(temp);
}

int FRTTransceiver::messagesOnQueue(eMultiSenderQueue multiSenderQueue)
{
   int pos = this->_getCommStruct(NULL,multiSenderQueue,false);

   if(pos == -1)
   {
      return -1;
   }

   if(this->_structCommPartners[pos].rxQueue == NULL)
   {
      return -1;
   }

   return this->_getAmountOfMessages(this->_structCommPartners[pos].rxQueue);
}

bool FRTTransceiver::hasDataFrom(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return false;
   }

   return this->_structCommPartners[pos].hasBufferedData;
}


int FRTTransceiver::amountOfBufferedDataFrom(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return -1;
   }

   return (this->_structCommPartners[pos].hasBufferedData ? this->_structCommPartners[pos].i8CurrTempcontainerPos + 1:0);
}

int FRTTransceiver::amountOfDataInAllBuffers()
{
   int amountOfDataAvail = 0;
   for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
   {
      if(this->_structCommPartners[u8I].hasBufferedData){
         amountOfDataAvail += this->_structCommPartners[u8I].i8CurrTempcontainerPos + 1;
      }
   }
   return amountOfDataAvail;
}


int FRTTransceiver::_getCommStruct(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
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
      if(!(multiSenderQueue >= 0 && multiSenderQueue < this->_u8MultiSenderQueues))
      {
         return -1;
      }

      uint8_t counter = 0;

      for(uint8_t u8I = 0; u8I < this->_u8CurrCommPartners;u8I++)
      {
         if(this->_structCommPartners[u8I].bReadOnlyCommunication)
         {
            if(multiSenderQueue == counter)
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


int FRTTransceiver::checkIfDataTypeInBuffer(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8Datatype)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return -1;
   }

   int counter = 0;
   if(this->_structCommPartners[pos].hasBufferedData)
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

const FRTTransceiver_TempDataContainer * FRTTransceiver::getNewestBufferedDataFrom(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return NULL;
   }

   if(this->_structCommPartners[pos].hasBufferedData)
   {
      return (const FRTTransceiver_TempDataContainer *)&this->_structCommPartners[pos].tempContainer[this->_structCommPartners[pos].i8CurrTempcontainerPos];
   }
   return NULL;
}


const FRTTransceiver_TempDataContainer * FRTTransceiver::getOldestBufferedDataFrom(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return NULL;
   }

   if(this->_structCommPartners[pos].hasBufferedData)
   {
      return (const FRTTransceiver_TempDataContainer *)&this->_structCommPartners[pos].tempContainer[0];
   }
   return NULL;
}

const FRTTransceiver_TempDataContainer * FRTTransceiver::getBufferedDataFrom(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar, 
                                                                                                                                   uint8_t u8PositionInBuffer)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);
   
   if(pos == -1)
   {
      return NULL;
   }

   if(this->_structCommPartners[pos].hasBufferedData && u8PositionInBuffer <= this->_structCommPartners[pos].i8CurrTempcontainerPos)
   {
      return (const FRTTransceiver_TempDataContainer *)&this->_structCommPartners[pos].tempContainer[u8PositionInBuffer];
   }
   
   return NULL;
}

void FRTTransceiver::addDataAllocateCallback(void(*fP)(const FRTTransceiver_DataContainerOnQueue &,FRTTransceiver_TempDataContainer &))
{
   this->_dataAllocator = fP;
}


void FRTTransceiver::addDataFreeCallback(void (*fP)(FRTTransceiver_TempDataContainer &))
{
   this->_dataDestroyer = fP;
}


string FRTTransceiver::_getPartnersName(FRTTransceiver_TaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar)
{
   int pos = this->_getCommStruct(partner,multiSenderQueue,bUseTaskHandleVar);

   if(pos == -1)
   {
      return string("");
   }

   return this->_structCommPartners[pos].partnersName;
}

int FRTTransceiver::_getAmountOfQueues(bool bTxQueue)
{
	int amount = 0;
	for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
	{
		if(bTxQueue)
		{
			if(!(this->_structCommPartners[u8I].txQueue == NULL)) amount++;
		}
		else
		{
			if(!(this->_structCommPartners[u8I].rxQueue == NULL)) amount++;
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
   /* General Infos */
   printf("General Infos\n\n");
   printf("\tOwner address           \t\t%p\n",this->_ownerAddress == NULL ? FRTTRANSCEIVER_UNKNOWNADDRESS : this->_ownerAddress);
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
      printf("\tAddress                 \t\t%p\n",this->_structCommPartners[u8I].commPartner == NULL ? FRTTRANSCEIVER_UNKNOWNADDRESS : this->_structCommPartners[u8I].commPartner);

      if(!this->_structCommPartners[u8I].bReadOnlyCommunication)
      {
         printf("\tComm-Type               \t\t%s\n",this->_ownerAddress == this->_structCommPartners[u8I].commPartner ? FRTTRANSCEIVER_COMMTYPE3:FRTTRANSCEIVER_COMMTYPE1);
      }
      else
      {
         printf("\tComm-Type               \t\t%s\n",FRTTRANSCEIVER_COMMTYPE2);
      }
      printf("\tTX-LINE                 \t\t%s\n",this->_structCommPartners[u8I].txQueue == NULL ? "OFF":"ON");
      printf("\t\tLength                %d\n",this->_structCommPartners[u8I].u8TxQueueLength);
      printf("\tRX-LINE                 \t\t%s\n",this->_structCommPartners[u8I].rxQueue == NULL ? "OFF":"ON");
      printf("\t\tLength                %d\n",this->_structCommPartners[u8I].u8RxQueueLength);
      printf("\tPackages sent           \t\t%d\n",this->_structCommPartners[u8I].dataPackagesSent);
      printf("\tPackages received       \t\t%d\n",this->_structCommPartners[u8I].dataPackagesReceived);
      printf("\tHas buffered data       \t\t%s\n",this->_structCommPartners[u8I].hasBufferedData ? "YES":"NO");
   }
}

#endif