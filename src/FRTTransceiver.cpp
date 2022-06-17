#ifndef FRTTRANSCEIVER_CPP
#define FRTTRANSCEIVER_CPP

/*!
 * \file       	FRTTransceiver.cpp
 * \brief      	Transceiver class methods implementation
 * \author     	Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version 	   v1.2.0
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

		/* Handle should not be nullptr AND no communication structure should be linked to the new partner */
		if(partner != nullptr && this->_getCommStruct(partner,eMultiSenderQueue::eNOMULTIQSELECTED,true) == -1)
		{
			this->_structCommPartners[_u8CurrCommPartners].commPartner = partner;
		}
		else
		{
			return false;
		}


		if(queueRX != nullptr)
		{	
			if(this->_queueExists(queueRX,false) != 1 && this->_queueExists(queueRX,true) != 1)
			{	
				if(semaphoreRx == nullptr || this->_semaphoreExists(semaphoreRx,true) == 1 	|| this->_semaphoreExists(semaphoreRx,false) == 1
																							|| (this->_checkValidQueueLength(u8QueueLengthRx) == false))
				{	
					/* It is possible to establish a connection without a queue (task-notification), 
					but since the queue was valid, we shouldnt procceed further because the user clearly wanted to add a queue */
					#ifdef LOG_INFO
					printf("Queue is valid but the semaphore OR queuelength is invalid!\n");
					#endif
					return false;
				}
				this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
				this->_structCommPartners[_u8CurrCommPartners].u8RxQueueLength = u8QueueLengthRx;
				this->_structCommPartners[_u8CurrCommPartners].semaphoreRxQueue = semaphoreRx;
			}
			else
			{	
				/* It is possible to establish a connection without a queue (task-notification), 
					but since the queue was already added to a comm-line, we shouldnt procceed further because the user clearly wanted to add a queue */
				#ifdef LOG_INFO
				printf("Queue already exists\n");
				#endif
				return false;
			}
		}


		if(queueTX != nullptr)
		{	
			if(this->_queueExists(queueTX,true) != 1 && this->_queueExists(queueTX,false) != 1)
			{
				if(semaphoreTx == nullptr || this->_semaphoreExists(semaphoreTx,true) == 1 	|| this->_semaphoreExists(semaphoreTx,false) == 1
																							|| (this->_checkValidQueueLength(u8QueueLengthTx) == false))
				{	
					/* It is possible to establish a connection without a queue (task-notification), 
					but since the queue was valid, we shouldnt procceed further because the user clearly wanted to add a queue */
					#ifdef LOG_INFO
					printf("Queue is valid but the semaphore OR queuelength is invalid!\n");
					#endif
					return false;
				}
				this->_structCommPartners[_u8CurrCommPartners].txQueue = queueTX;
				this->_structCommPartners[_u8CurrCommPartners].u8TxQueueLength = u8QueueLengthTx;
				this->_structCommPartners[_u8CurrCommPartners].semaphoreTxQueue = semaphoreTx;
			}
			else
			{	
				/* It is possible to establish a connection without a queue (task-notification), 
					but since the queue was already added to a comm-line, we shouldnt procceed further because the user clearly wanted to add a queue */
				#ifdef LOG_INFO
				printf("Queue already exists\n");
				#endif
				return false;
			}
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
			if(this->_queueExists(queueRX,false) != 1 && this->_queueExists(queueRX,true) != 1)
			{
				if(semaphoreRx == nullptr || this->_semaphoreExists(semaphoreRx,true) == 1 	|| this->_semaphoreExists(semaphoreRx,false) == 1
																							||(this->_checkValidQueueLength(u8QueueLengthRx) == false))
				{	
					/* It is possible to establish a connection without a queue (task-notification), 
					but since the queue was valid, we shouldnt procceed further because the user clearly wanted to add a queue */
					#ifdef LOG_INFO
					printf("Queue is valid but the semaphore OR queuelength is invalid!\n");
					#endif
					return false;
				}
				this->_structCommPartners[_u8CurrCommPartners].rxQueue = queueRX;
				this->_structCommPartners[_u8CurrCommPartners].u8RxQueueLength = u8QueueLengthRx;
				this->_structCommPartners[_u8CurrCommPartners].semaphoreRxQueue = semaphoreRx;
			}
			else
			{	
				/* It is possible to establish a connection without a queue (task-notification), 
					but since the queue was already added to a comm-line, we shouldnt procceed further because the user clearly wanted to add a queue */
				#ifdef LOG_INFO
				printf("Queue already exists\n");
				#endif
				return false;
			}
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
	int FRTTransceiver::_queueExists(FRTTQueueHandle queue,bool bTxQueue)
	{
		if(!queue) return -1;

		for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
		{
			FRTTQueueHandle toCheck = (bTxQueue ? this->_structCommPartners[u8I].txQueue:this->_structCommPartners[u8I].rxQueue);
			
			if(toCheck == queue) return 1;
		}

		return 0;
	}

	int FRTTransceiver::_semaphoreExists(FRTTSemaphoreHandle smph,bool bTxSemaphore)
	{
		if(!smph) return -1;

		for(uint8_t u8I = 0;u8I < this->_u8CurrCommPartners;u8I++)
		{
			SemaphoreHandle_t toCheck = (bTxSemaphore ? this->_structCommPartners[u8I].semaphoreTxQueue:this->_structCommPartners[u8I].semaphoreRxQueue);
			
			if(toCheck == smph) return 1;
		}

		return 0;
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

	bool FRTTransceiver::NotifyBasic(FRTTTaskHandle partner)
	{
		int pos = this->_getCommStruct(partner,eMultiSenderQueue::eNOMULTIQSELECTED,true);

		/* check for bReadOnlyComm..(true) unnecessary since commPartner will be nullptr for readOnlyQueues anyways*/
		if(pos == -1 || this->_structCommPartners[pos].commPartner == nullptr || this->_structCommPartners[pos].bReadOnlyCommunication == true)
		{

			#ifdef LOG_INFO
			printf("You are not to notify your partner \nOne of the following things happened:\n"
				"-[no communication struct available]\n"
				"-[communication partner address == nullptr AND/OR communication line is a multiSenderQueue]\n");
			#endif
			return false;
		}

		/**
		 *	From here on we will be fine UNLESS the user provided an invalid FRTTTaskhandle! 
		 */
		
		xTaskNotifyGive(this->_structCommPartners[pos].commPartner);	/* WILL ALWAYS RETURN pdPASS*/

		#ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
		this->_tasksNotified++;
		#endif
		
		return true;
	}

	bool FRTTransceiver::NotifyExtended(FRTTTaskHandle partner,eFRTTNotifyActions action,uint32_t u32NotificationMask)
	{
		int pos = this->_getCommStruct(partner,eMultiSenderQueue::eNOMULTIQSELECTED,true);

		/* check for bReadOnlyComm..(true) unnecessary since commPartner will be nullptr for readOnlyQueues anyways*/
		if(pos == -1 || this->_structCommPartners[pos].commPartner == nullptr || this->_structCommPartners[pos].bReadOnlyCommunication == true)
		{

			#ifdef LOG_INFO
			printf("You are not to notify your partner \nOne of the following things happened:\n"
				"-[no communication struct available]\n"
				"-[communication partner address == nullptr AND/OR communication line is a multiSenderQueue]\n");
			#endif
			return false;
		}

		if(!(action >= eFRTTNotifyActions::e_NoAction && action <= eFRTTNotifyActions::e_SetValueWithoutOverwrite))
		{
			#ifdef LOG_INFO
			printf("You are not to notify your partner \nOne of the following things happened:\n"
					"-[eFRTTNotifyAction is invalid in this case. Possible interval: [e_NoAction;e_SetValueWithoutOverwrite]]\n");
			#endif
			return false;
		}

		/**
		 *	From here on we will be fine UNLESS the user provided an invalid FRTTTaskhandle! 
		 */


		/*

				---> FREERTOS ENUM

			typedef enum
			{
				eNoAction = 0,                
				eSetBits,                     
				eIncrement,                   
				eSetValueWithOverwrite,       
				eSetValueWithoutOverwrite     
			} eNotifyAction;

		*/

		eNotifyAction freeRtosAction;//= (eNotifyAction)(static_cast<uint8_t>(action) - 2);

		switch(action)
		{
			case eFRTTNotifyActions::e_NoAction:
				freeRtosAction = eNoAction;
				break;
			case eFRTTNotifyActions::e_SetBits:
				freeRtosAction = eSetBits;
				break;
			case eFRTTNotifyActions::e_Increment:
				freeRtosAction = eIncrement;
				break;
			case eFRTTNotifyActions::e_SetValueWithOverwrite:
				freeRtosAction = eSetValueWithOverwrite;
				break;
			case eFRTTNotifyActions::e_SetValueWithoutOverwrite:
				freeRtosAction = eSetValueWithoutOverwrite;
				break;
			case eFRTTNotifyActions::e_CLEARCOUNTONEXIT:
			case eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT:
			default:
				/* Wont ever come to this situation because we check it in the if() above (e_CLEARCOUNTON***** not allowed here). 
														Just so that the c++ compiler for the esp8266 doesnt throw -Werror=switch since we define more enumerators than freertos..!*/
				freeRtosAction = eNoAction; /* randomly chosen */
				break;
		}
		FRTTBaseType retVal =  xTaskNotify(partner,u32NotificationMask,freeRtosAction);

		#ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
		if(retVal == pdTRUE) this->_tasksNotified++;
		#endif

		return (retVal == pdFAIL) ? false:true;
	}

	FRTTransceiver &  FRTTransceiver::NotifyReceiveBasic(eFRTTNotifyActions action,int blockTimeReceive_Ms)
	{

		blockTimeReceive_Ms = this->_checkWaitTime(blockTimeReceive_Ms);
		
		if(blockTimeReceive_Ms == -2 || !(action == eFRTTNotifyActions::e_CLEARCOUNTONEXIT || action == eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT))
		{  
			return *this;
		}

		unsigned long timeToWaitReceive = (blockTimeReceive_Ms == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(blockTimeReceive_Ms));
		
		FRTTBaseType clearCount = (action == eFRTTNotifyActions::e_CLEARCOUNTONEXIT ? pdTRUE:pdFALSE);

		uint32_t u32TempNotificationVal = ulTaskNotifyTake(clearCount,(TickType_t)timeToWaitReceive);
		
		/* 
		   FreeRTOS lets the task wait in the blocked state until the notification value != 0 
		   So value > 0 == notfication received.
		*/

		#ifdef LOG_INFO
		if(!u32TempNotificationVal) printf("NotifyReceiveBasic: Nothing received!\n");
		#endif

		#ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
		if(u32TempNotificationVal != 0) this->_notificationsReceived++;
		#endif

		this->_bHasNotification = (u32TempNotificationVal != 0 ? true:false);

		this->_u32NotificationValue = u32TempNotificationVal;
		return *this;
	}

	FRTTransceiver & FRTTransceiver::NotifyReceiveExtended(uint32_t u32ClearOnEntryMask,uint32_t u32ClearOnExitMask,int blockTimeReceive_Ms)
	{
		int timeToWaitReceive = this->_checkWaitTime(blockTimeReceive_Ms);

		if(timeToWaitReceive == -2)
		{
			return *this;
		}

		timeToWaitReceive = (timeToWaitReceive == FRTTRANSCEIVER_WAITMAX ? portMAX_DELAY : pdMS_TO_TICKS(timeToWaitReceive));

		FRTTBaseType retVal = xTaskNotifyWait(u32ClearOnEntryMask,u32ClearOnExitMask,&this->_u32NotificationValue,timeToWaitReceive);

		if(retVal == pdTRUE)
		{	
			this->_bHasNotification = true;
			#ifdef LOG_INFO
			printf("Notfication received\n");
			#endif

			#ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
			this->_notificationsReceived++;
			#endif
		}
		else
		{	
			this->_bHasNotification = false;
			#ifdef LOG_INFO
			printf("Nothing received\n");
			#endif
		}

		return *this;
	}

	uint32_t FRTTransceiver::getNotificationVal()
	{
		return this->_u32NotificationValue;
	}

	void FRTTransceiver::clearNotificationVal()
	{
		this->_u32NotificationValue = 0;
	}

	bool FRTTransceiver::hasNotification()
	{
		return this->_bHasNotification;
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

		auto runtime = std::chrono::high_resolution_clock::now() - this->_runtimeStart;
		std::chrono::minutes min = std::chrono::duration_cast<std::chrono::minutes>(runtime);

		printf("General Infos\n\n");
		printf("\tOwner address            \t\t%p\n",this->_ownerAddress == nullptr ? FRTTRANSCEIVER_UNKNOWNADDRESS : this->_ownerAddress);
		printf("\tCommunicationpartner     \t\t(%d out of %d)\n",this->_u8CurrCommPartners,this->_u8MaxPartners);
		printf("\t\t- - - > (%d of those read only)\n",this->_u8MultiSenderQueues);
		printf("\tMax partners             \t\t%d\n",this->_u8MaxPartners);
		printf("\tTX-QUEUE CONNECTIONS	   \t\t%d\n",this->_getAmountOfQueues(true));
		printf("\tRX-QUEUES CONNECTIONS	   \t\t%d\n",this->_getAmountOfQueues(false));
		printf("\tData callbacks available \t\t%s\n",this->_hasDataInterpreters() ? "yes":"no");
		printf("\tBroadcasts made          \t\t%d\n",this->_broadcastCount);
		printf("\tNotifications received   \t\t%d\n",this->_notificationsReceived);
		printf("\tNotifications sent   	   \t\t%d\n",this->_tasksNotified);
		printf("\tLast notification value  \t\t%ld\n\n",(long int)this->getNotificationVal());
		if(min.count() > 0)
		{
			printf("\tRunning for [%ld min]\n\n",(long int)min.count());
		}
		else
		{	
			std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(runtime);
			printf("\tRunning for [%ld sec]\n\n",(long int)sec.count());
		}
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
		printf("\n\n");
	}
	#endif

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

			#ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
			this->_runtimeStart = std::chrono::high_resolution_clock::now();
			#endif
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