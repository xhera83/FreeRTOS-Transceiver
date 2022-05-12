/*!
 * \file       FRTTransceiver.cpp
 * \brief      Transceiver class methods
 * \author     Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <FRTTransceiver.h>

//#define LOG_INFO
#define LOG_WARNING

FRTTransceiver_QueueHandle FRTTransceiver_CreateQueue(FRTTransceiver_BaseType lengthOfQueue,FRTTransceiver_BaseType elementSize)
{
   FRTTransceiver_QueueHandle queue = xQueueCreate(lengthOfQueue,elementSize);

   if(!queue)
   {
      #ifdef LOG_WARNING
      log_w("Queue cannot be created [Insufficient heap memory]\n");
      #endif
   }
   else
   {
      #ifdef LOG_INFO
      log_i("Queue successfully created. Queue handle returned\n");
      #endif
   }
   return queue;
}


FRTTransceiver_SemaphoreHandle FRTTransceiver_CreateSemaphore()
{
   FRTTransceiver_SemaphoreHandle semaphore = xSemaphoreCreateMutex();

   if(!semaphore)
   {
      #ifdef LOG_WARNING
      log_w("Semaphore cannot be created [Insufficient heap memory]\n");
      #endif
   }
   else
   {
      #ifdef LOG_INFO
      log_i("Semaphore successfully created. Semaphore handle returned\n");
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

bool FRTTransceiver::_hasSemaphore(FRTTransceiver_TaskHandle partner)
{
   int pos;
   if((pos  = this->_getCommStruct(partner)) == -1)
   {
      return false;
   }

   if(this->_structCommPartners[pos].semaphore)
   {
      return true;
   }
   else
   {
      return false;
   }
}


FRTTransceiver::FRTTransceiver(uint8_t u8MaxPartners)
{
   this->_u8MaxPartners = u8MaxPartners;
   this->_structCommPartners = new struct CommunicationPartner[u8MaxPartners];
}

FRTTransceiver::~FRTTransceiver()
{
   delete[] this->_structCommPartners;
}


bool FRTTransceiver::addCommPartner(FRTTransceiver_TaskHandle partnersAddress,FRTTransceiver_SemaphoreHandle semaphore,FRTTransceiver_QueueHandle queueRX,FRTTransceiver_QueueHandle queueTX,const string partnersName)
{

   if(this->_u8CurrCommPartners + 1 > this->_u8MaxPartners)
   {
      return false;
   }

   if(partnersAddress != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].commPartner = partnersAddress;
   }
   else
   {
      return false;
   }

   if(semaphore != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].semaphore = semaphore;
   }
   else
   {
      return false;
   }

   if(queueRX != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
   }

   if(queueTX != NULL)
   {
      this->_structCommPartners[_u8CurrCommPartners].txQueue = queueTX;
   }

   this->_structCommPartners[_u8CurrCommPartners].partnersName = partnersName;

   this->_u8CurrCommPartners++;
   return true;
}



#if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
bool FRTTransceiver::writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8DataType,void * data,int blockTimeWrite,int blockTimeTakeSemaphore,uint32_t u32AdditionalData)
#elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
bool FRTTransceiver::writeToQueue(FRTTransceiver_TaskHandle destination,uint8_t u8DataType,void * data,int blockTimeWrite,int blockTimeTakeSemaphore,uint64_t u64AdditionalData)
#endif
{
   int pos;

   if(!this->_hasDataInterpreters() || !this->_hasSemaphore(destination) || (pos  = this->_getCommStruct(destination)) == -1)
   {
      #ifdef LOG_WARNING
      log_e("You are not allowed to write to a queue \nOne of the following things happened:\n"
            "-[no callback functions for (allocating,freeing) data supplied]\n"
            "-[no semphores supplied]"
            "-[destination task unknown]\n");
      #endif
      return false;
   }

   if(this->_structCommPartners[pos].txQueue == NULL || data == NULL)
   {
      #ifdef LOG_WARNING
      log_w("Action now allowed \nOne of the following things happened:\n"
            "-[no tx queue supplied]"
            "-[data pointer null]\n");
      #endif
      return false;
   }

   this->_structCommPartners[pos].txLineContainer =
   {
      .data = data,
      .u8DataType = u8DataType,
      #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
      .u32AdditionalData = u32AdditionalData,
      #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
      .u64AdditionalData = u64AdditionalData,
      #endif
   };

   unsigned long timeToWait = _checkWaitTime(blockTimeTakeSemaphore);

   if(timeToWait == -2)
   {  
      return false;
   }

   timeToWait = (timeToWait == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWait));

   SemaphoreHandle_t s = this->_structCommPartners[pos].semaphore;

   if(xSemaphoreTake(s,timeToWait) == pdFALSE)
   {
      #ifdef LOG_INFO
      log_i("Semaphore was not available before block time expired.")
      #endif
      return false;
   }

   timeToWait = _checkWaitTime(blockTimeWrite);

   if(timeToWait == -2)
   {
      xSemaphoreGive(s);
      return false;
   }

   timeToWait = (timeToWait == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWait));

   
   FRTTransceiver_BaseType returnVal = xQueueSendToBack(this->_structCommPartners[pos].txQueue,(const void *)&this->_structCommPartners[pos].txLineContainer,(TickType_t)timeToWait);

   xSemaphoreGive(s);

   if(!(returnVal == pdTRUE))
   {
      return false;
   }

   return true;
}


bool FRTTransceiver::addCommQueue(FRTTransceiver_TaskHandle partner, FRTTransceiver_QueueHandle queueRxOrTx,bool isTxQueue)
{
   int pos;

   if((pos = this->_getCommStruct(partner)) == -1 || queueRxOrTx == NULL)
   {
      return false;
   }

   CommunicationPartner & temp = this->_structCommPartners[pos];

   if(isTxQueue == true)
   {
      
      temp.txQueue = queueRxOrTx;
   }
   else
   {
      temp.rxQueue = queueRxOrTx;
   }

   return true;
}


bool FRTTransceiver::readFromQueue(FRTTransceiver_TaskHandle source,int blockTimeRead,int blockTimeTakeSemaphore)
{

   if(!this->_hasDataInterpreters() || !this->_hasSemaphore(source))
   {
      return false;
   }

   int pos;

   if((pos = this->_getCommStruct(source)) == -1 || this->_structCommPartners[pos].rxQueue == NULL)
   {
      return false;

   }

   unsigned long timeToWait = this->_checkWaitTime(blockTimeTakeSemaphore);

   if(timeToWait == -2)
   {
      return false;
   }

   timeToWait = (timeToWait == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWait));

   SemaphoreHandle_t s = this->_structCommPartners[pos].semaphore;

   if(xSemaphoreTake(s,timeToWait) == pdFALSE)
   {
      return false;
   }

   if(this->_structCommPartners[pos].hasData)
   {
      this->_dataDestroyer(this->_structCommPartners[pos].tempContainer);
      this->_structCommPartners[pos].hasData = false;
   }

   timeToWait = this->_checkWaitTime(blockTimeRead);

   if(timeToWait == -2)
   {
      xSemaphoreGive(s);
      return false;
   }

   timeToWait = (timeToWait == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWait));


   

   FRTTransceiver_BaseType returnVal = xQueueReceive(this->_structCommPartners[pos].rxQueue,(void *)&this->_structCommPartners[pos].rxLineContainer,(TickType_t)timeToWait);

   if(!(returnVal == pdTRUE))
   {
      xSemaphoreGive(s);
      return false;
   }

   this->_dataAllocator(this->_structCommPartners[pos].rxLineContainer,this->_structCommPartners[pos].tempContainer);
   this->_structCommPartners[pos].hasData = true;
   
   xSemaphoreGive(s);
   return true;
}



void FRTTransceiver::manualDeleteAllocatedData(FRTTransceiver_TaskHandle partner)
{
   int pos;
   if(!this->_hasDataInterpreters() || (pos = this->_getCommStruct(partner)) == -1)
   {
      return;
   }

   if(this->_structCommPartners[pos].hasData)
   {
      #ifdef LOG_INFO
      log_i("Manually deleting allocated data");
      #endif
      this->_dataDestroyer(this->_structCommPartners[pos].tempContainer);
      this->_structCommPartners[pos].hasData = false;
      return;
   }
}


void FRTTransceiver::manualDeleteAllAllocatedData()
{
   if(!this->_hasDataInterpreters())
   {
      return;
   }
   for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
   {
      if(this->_structCommPartners[u8I].hasData)
      {
         #ifdef LOG_INFO
         log_i("Deleting data for partner %li tx line",this->structCommPartners[u8I].commPartner);
         #endif
         this->_dataDestroyer(this->_structCommPartners[u8I].tempContainer);
         this->_structCommPartners[u8I].hasData = false;
      }
   }
}


int FRTTransceiver::messagesOnQueue(FRTTransceiver_TaskHandle partner)
{
   int pos;

   if((pos = this->_getCommStruct(partner)) == -1)
   {
      return -1;
   }
   return this->_getAmountOfMessages(this->_structCommPartners[pos].rxQueue);
}


bool FRTTransceiver::hasDataFrom(FRTTransceiver_TaskHandle partner)
{
   int pos;

   if((pos = this->_getCommStruct(partner)) == -1)
   {
      return false;
   }

   return this->_structCommPartners[pos].hasData;
}


int FRTTransceiver::waitingData()
{
   int amountOfDataAvail = 0;
   for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
   {
      if(this->_structCommPartners[u8I].hasData) amountOfDataAvail++;
   }
   return amountOfDataAvail;
}


int FRTTransceiver::_getCommStruct(FRTTransceiver_TaskHandle partner)
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




const TempDataContainer * FRTTransceiver::getNewestDataFrom(FRTTransceiver_TaskHandle partner)
{
   int pos = this->_getCommStruct(partner);

   if(pos == -1)
   {
      return NULL;
   }

   return (const TempDataContainer *)&this->_structCommPartners[pos].tempContainer;
}


void FRTTransceiver::addDataAllocateCallback(void(*funcPointerCallback)(const DataContainerOnQueue &,TempDataContainer &))
{
   this->_dataAllocator = funcPointerCallback;
}


void FRTTransceiver::addDataFreeCallback(void (*funcPointerCallback)(TempDataContainer &))
{
   this->_dataDestroyer = funcPointerCallback;
}


string FRTTransceiver::getPartnersName(FRTTransceiver_TaskHandle partner)
{
   int pos = this->_getCommStruct(partner);

   if(pos == -1)
   {
      return NULL;
   }

   return this->_structCommPartners[pos].partnersName;
}


int FRTTransceiver::_checkWaitTime(int timeMs)
{
   /* WAITMAX is defined as -1, If timeMs element of [-infinite;0[, than display error! */
   if(timeMs < FRTTRANSCEIVER_WAITMAX || (timeMs > FRTTRANSCEIVER_WAITMAX && timeMs < 0))
   {
      #ifdef LOG_WARNING
      log_w("Nothing sent [invalid wait time specified]\n");
      #endif
      return -2;
   }
   return timeMs;
}
