#ifndef FRTTRANSCEIVER_H
#define FRTTRANSCEIVER_H

/*!
 * \file        FRTTransceiver.h
 * \brief       Class definition headerfile
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version     v1.1.0
 * \copyright   Copyright 2022 Xhemail Ramabaja
 */

#include "sdkconfig.h"

#if !(ESP32 || ESP8266 || CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP8266)
#error "FreeRTOS-Transceiver is not usable with this MCU."
#endif 

#include "FRTTransceiverNotify.h"
#include "FRTTransceiverExtension.h"
#include <string>
#include <chrono>

using namespace std;

namespace FRTT {
    #if defined(ESP32) || defined (CONFIG_IDF_TARGET_ESP32)
        /*! 
        * \brief                         Function for task-creation (ESP32)
        * \param    taskName             Name of the task
        * \param    stackbytes           Stack size in BYTES
        * \param    taskParameter        Pointer to data that is being passed to the task
        * \param    taskPriority         Priority of the task (Max priority for the ESP32 == configMAX_PRIORITIES)
        * \param    taskHandle           Address of the FRTTTaskhandle, which is holding the address to the task control block
		* \param 	core				 CPU CORE (possible values: 0 and 1)
        * \return   void                                      
        */
        void FRTTCreateTask(FRTTTaskFunction taskCode,
                            const char * const taskName,
                            const uint32_t stackbytes,
                            void * const taskParameter,
                            FRTTBaseType taskPriority,
                            FRTTTaskHandle * taskHandle,
                            const FRTTBaseType core);
    #elif defined(ESP8266) || defined(CONFIG_IDF_TARGET_ESP8266)
        /*! 
        * \brief                         Function for task-creation (ESP8266)
        * \param    taskName             Name of the task
        * \param    stackbytes           Stack size in BYTES
        * \param    taskParameter        Pointer to data that is being passed to the task
        * \param    taskPriority         Priority of the task (Max priority for the ESP8266 == configMAX_PRIORITIES)
        * \param    taskHandle           Address to the task control block
        * \return   void                                      
        */
        void FRTTCreateTask(FRTTTaskFunction taskCode,
                            const char * const taskName,
                            const uint16_t stackbytes,
                            void * const taskParameter,
                            FRTTBaseType taskPriority,
                            FRTTTaskHandle * taskHandle);
    #endif

    /*! 
    * \brief                       Creates a queue
    * \param   lengthOfQueue       Holds the desired queuelength
    * \return                      ::FRTTQueueHandle or nullptr                                      
    */
    FRTTQueueHandle FRTTCreateQueue(FRTTBaseType lengthOfQueue);
    /*! 
    * \brief                       Creates a semaphore
    * \return                      Address to the semaphore or nullptr
    * \note                        The user needs to create one for the tx-line and one for the rx-line                                   
    */
    FRTTSemaphoreHandle FRTTCreateSemaphore();

    /*!
    * \brief   Class definition
    */
    class FRTTransceiver
    {
        private:                                                                 
            FRTTTaskHandle _ownerAddress = nullptr;                                 /*!< Address of the task owning this object     */                                           
            struct FRTTCommunicationPartner * _structCommPartners = nullptr;        /*!< Array of all connections                   */
            uint8_t _u8CurrCommPartners = 0;                                        /*!< Amount of communications connected to      */
            uint8_t _u8MaxPartners = 0;                                             /*!< Max amount of possible connections         */
            uint8_t _u8MultiSenderQueues = 0;                                       /*!< Amount of multi-sender-queues (multiple tasks write on the tx line)            */
			uint32_t _u32NotificationValue = 0;										/*!< Holds future notification values of the owner task 							*/
            bool _bHasNotification = false;                                         /*!< Signals whether a notfication was received or not                              */
            bool _bDelete = false;                                                  /*!< Signals whether delete [] is needed in the destructor                          */
            bool _bHasValidStruct = false;                                          /*!< Signals whether ::_structCommPartners is NOT nullptr || _u8MaxPartners != 0    */

            #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
            int _broadcastCount = 0;                                                /*!< Amount of broadcasts made. Important for FRTTransceiver::printCommunicationsSummary() */
            int _notificationsReceived = 0;                                         /*!< Amount of notifications received. Important for FRTTransceiver::printCommunicationsSummary() */
            int _tasksNotified = 0;                                                 /*!< Amount of tasks notified. Important for FRTTransceiver::printCommunicationsSummary() */
            std::chrono::high_resolution_clock::time_point _runtimeStart;
            #endif

            fP_dataAllocateCallback _dataAllocator = nullptr;                       /*!< Function pointer to the data allocator callback supplied by the user       */
            fP_dataFreeCallback _dataDestroyer = nullptr;                           /*!< Function pointer to the data de-allocator callback supplied by the user    */

            /*! 
            * \brief                        Returns the amount of queues (rx or tx)
            * \param bTxQueue               Signals whether to return the amount of rx or tx queues          
            * \return                       The amount of queues (rx or tx) or -1 (if no communication struct available)                          
            */
            int _getAmountOfQueues(bool bTxQueue);

            /*! 
            * \brief                        Checks if the supplied length of one queue is valid
            * \param u8QueueLength          Length of the queue
            * \return                       True if the length is valid                           
            */
            bool _checkValidQueueLength(uint8_t u8QueueLength);
            /*! 
            * \brief                        Checks if the supplied wait-time for read/write operations is valid
            * \param time_ms                Wait-time in milliseconds
            * \return                       Wait-time in milliseconds or -2 (-2 because #FRTTRANSCEIVER_WAITMAX is set to -1)                           
            */
            int _checkWaitTime(int time_ms);
            /*! 
            * \brief                        Rearranges elements inside the internal buffers around (after one element was removed)
            * \param u8CommStructPos        Position of a FRTTCommunicationPartner struct in #_structCommPartners, holding the buffer that needs rearrangement
            * \param u8PosRemoved           Position in buffer that was removed
            * \return                       void                           
            */
            void _rearrangeTempContainerArray(uint8_t u8CommStructPos,uint8_t u8PosRemoved);
            /*! 
            * \brief                        Returns the position of the FRTTCommunicationPartner structure
            * \param partner                Used to select the right entry in FRTTCommunicationPartner
            * \param multiSenderQueue       Used to select the right entry in FRTTCommunicationPartner
            * \param bUseTaskHandleVar      Signals whether to use the partner parameter or the multiSenderQueue parameter
            * \return                       Position or -1  
            * \note                         Here either FRTTTaskHandle or an eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false
            */
            int _getCommStruct(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
            /*! 
            * \brief                        Checks if a semaphore is available for the selected line (rx/tx)
            * \param partner                Used to select the right entry in FRTTCommunicationPartner
            * \param multiSenderQueue       Used to select the right entry in FRTTCommunicationPartner
            * \param bUseTaskHandleVar      Signals whether to use the partner parameter or the multiSenderQueue parameter
            * \param txLine                 Signals for which semaphore to check
            * \return                       True if semaphore available  
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false
            */
            bool _hasSemaphore(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar, bool bTxLine);
            /*! 
            * \brief                        Checks if there are messages on the queue
            * \param queue                  Address of the queue to check
            * \return                       True if at least one message on the queue                         
            */
            bool _checkForMessages(FRTTQueueHandle queue);
            /*! 
            * \brief                        Checks if 'queue' has already been added to a communication line
            * \param queue                  Address of the queue to look for
            * \param bTxQueue               Signals what type of queue 'queue' is (rx|tx)
            * \return                       Returns 1 if queue has already been added<br>
            *                               Returns 0 if queue has not been added yet<br>
            *                               Returns -1 if 'queue' is nullptr                        
            */
            int _queueExists(FRTTQueueHandle queue,bool bTxQueue);
            /*! 
            * \brief                        Checks if 'smph' has already been added to a communication line
            * \param queue                  Address of the semaphore to find
            * \param bTxQueue               Signals what type of semaphore 'smph' is (semaphore of a rx|tx queue)
            * \return                       Returns 1 if semaphore has already been added<br>
            *                               Returns 0 if semaphore has not been added yet<br>
            *                               Returns -1 if 'smph' is nullptr                        
            */
            int _semaphoreExists(FRTTSemaphoreHandle smph,bool bTxSemaphore);
            /*! 
            * \brief                        Returns the amount of messages on the queue
            * \param queue                  Address of the queue to check
            * \return                       Amount of messages or -1                         
            */
            int _getAmountOfMessages(FRTTQueueHandle queue);
            /*! 
            * \brief                        Checks if user supplied neccessary callback functions
            * \return                       True if callbacks available                           
            */
            bool _hasDataInterpreters();
            /*! 
            * \brief                        Returns the partners name
            * \param partner                Used to select the right entry in FRTTCommunicationPartner
            * \param multiSenderQueue       Used to select the right entry in FRTTCommunicationPartner
            * \param bUseTaskHandleVar      Signals whether to use the partner parameter or the multiSenderQueue parameter
            * \return                       Partners name or empty string  
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false
            */
            string _getPartnersName(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
        public:
            /*! 
            * \brief                        FRTTransceiver Constructor (Deleted)
            * \note                         Will be removed since there are currently no options to add crucial data with class-methods at a later time after instantiation<br>
            *                               So the user is therefore promted to use the other constructors instead and supply important data right away.                      
            */
            FRTTransceiver() = delete;
            /*! 
            * \brief                        FRTTransceiver Constructor (1)
            * \details                      Allocates memory for FRTTCommunicationPartner structure (memory will be automatically released when instance<br> 
            *                               goes out of scope or implicit FRTTransceiver::~FRTTransceiver() called)
            * \param ownerAddress           Address of the task owning this object (can be nullptr, other tasks wont know who the source of messages is)
            * \param u8MaxPartners          Max possible connections
			* \note      				    Owner address will only be used to decorate tx packages with a source address (Task notification will know the address of this object)
            * \attention                    If u8MaxPartners is 0, then the constructor will internally increment it to 1 (min 1 partner)!             
            */
            FRTTransceiver(FRTTTaskHandle ownerAddress,uint8_t u8MaxPartners);
            /*! 
            * \brief                        FRTTransceiver Constructor (2)
            * \details                      Receives a pre-declared array of FRTTCommunicationPartner
            * \param ownerAddress           Address of the task owning this object (can be null, other tasks wont know who the source of messages is)
            * \param u8MaxPartners          Max possible connections
            * \param commStructs            Pointer to the pre-declared array of FRTTCommunicationPartner.
			* \note      				    Owner address will only be used to decorate tx packages with a source address (Task notification will know the address of this object)   
            * \attention                    If commStructs is a nullptr OR u8MaxPartners is 0, then all class methods wont work<br>
            *                               Supplying the wrong u8MaxPartners to the commStruct will result in undefined library behaviour (so make sure array size and u8MaxPartners match!)                  
            */
            FRTTransceiver(FRTTTaskHandle ownerAddress,FRTTCommunicationPartner * commStructs,uint8_t u8MaxPartners):   _ownerAddress(ownerAddress),
                                                                                                                        _structCommPartners(commStructs),
                                                                                                                        _u8MaxPartners(u8MaxPartners),
                                                                                                                        _bDelete(false),
                                                                                                                        _bHasValidStruct(commStructs && u8MaxPartners ? true:false)
                                                                                                                        {};
            /*! 
            * \brief                        Destructor
            * \details                      Will free memory previously allocated by the constructor (If needed)
            */
            ~FRTTransceiver();
            /*! 
            * \brief                        Adds a new communication to the list
            * \param partner                Address of the partner task
            * \param queueRX                Rx queue to the partner (reading messages from partner)
            * \param u8QueueLengthRx        Rx queue length
            * \param semaphoreRx            Rx semaphore
            * \param queueTX                Tx queue to the partner (writing to the partner)
            * \param u8QueueLengthTx        Tx queue length
            * \param semaphoreTx            Tx semaphore
            * \param partnersName           Partners name
			* \attention 					If you choose to provide an some random invalid address as the partner task, please make sure not to call for any task-notification 'notify task' methods!<br>
            *                               There is currently no solution available to check if a FRTTTaskhandle is valid so the correct execution of this method is in your hands<br>
            *                               An invalid FRTTTaskhandle will result in a FreeRTOS crash!
            *                                 
            * \return                       True if communication was added
            */
            bool addCommPartner(FRTTTaskHandle partner,FRTTQueueHandle queueRX,uint8_t u8QueueLengthRx,FRTTSemaphoreHandle semaphoreRx,FRTTQueueHandle queueTX,
                                                       uint8_t u8QueueLengthTx,FRTTSemaphoreHandle semaphoreTx,const string partnersName = string());
        
            /*! 
            * \brief                        Adds a new multi-sender-queue connection (multiple tasks write on the tx line)
            * \param queueRX                Rx queue to the x tasks writing on it
            * \param u8QueueLengthRx        Rx queue length
            * \param semaphoreRx            Rx semaphore
            * \param multiSenderQueueName   Multi-sender-queue name
            * \return                       True if communication was added
            * \note                         You can not add a tx queue to this communication, since there is no multi-reader-queue feature.<br>
            *                               This communication line will therefore be read-only
            *                               
            */
            bool addMultiSenderPartner(FRTTQueueHandle queueRX,uint8_t u8QueueLengthRx,FRTTSemaphoreHandle semaphoreRx,
                        const string multiSenderQueueName = string());

            #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
            /*! 
            * \brief                            Writes to a selected tx queue
            * \param destination                Destination queue (partner address)
            * \param u8Datatype                 Datatype of void pointer 'data'. (Create your own system wide datatypes)
            * \param data                       Pointer to the original data
            * \param blockTimeWrite_Ms          How long the task calling this method should be put into a blocked state (max) until it is allowed to write to the queue (milliseconds)
            * \param blockTimeTakeSemaphore_Ms  How long the task calling this method should be put into a blocked state (max) until is is allowed to aquire the semaphore for the operation (milliseconds)
            * \param u32AdditionalInfo          32 Bit additional payload along with void pointer 'data'       
            * \return                           True if it was successfully written to the tx queue
            * \note                             Here you can pass a 32 bit variable along with the data pointer. <br>
            *                                   You can have the same functionality but with an additional 64 bit payload instead, by uncommenting FRTTRANSCEIVER_64BITADDITIONALDATA
            *                                   in FRTTransceiverSettings.h
            */
            bool writeToQueue(FRTTTaskHandle destination,uint8_t u8Datatype,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint32_t u32AdditionalInfo);
            #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
            /*! 
            * \brief                            Writes to a selected tx queue
            * \param destination                Destination queue (partner address)
            * \param u8Datatype                 Datatype of void pointer 'data'. (Create your own system wide datatypes)
            * \param data                       Pointer to the original data
            * \param blockTimeWrite_Ms          How long the task calling this method should be put into a blocked state (max) until it is allowed to write to the queue (milliseconds)
            * \param blockTimeTakeSemaphore_Ms  How long the task calling this method should be put into a blocked state (max) until is is allowed to aquire the semaphore for the operation (milliseconds)
            * \param u64AdditionalInfo          64 Bit additional payload along with void pointer 'data'       
            * \return                           True if it was successfully written to the tx queue
            * \note                             Here you can pass a 64 bit variable along with the data pointer. <br>
            *                                   You can have the same functionality but with an additional 32 bit payload instead, by uncommenting FRTTRANSCEIVER_32BITADDITIONALDATA
            *                                   in FRTTransceiverSettings.h
            */
            bool writeToQueue(FRTTTaskHandle destination,uint8_t u8Datatype,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint64_t u64AdditionalData);
            #endif

            #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
            /*! 
            * \brief                            Broadcasts data to all communication lines with a tx connection
            * \param u8Datatype                 Datatype of void pointer 'data'. (Create your own system wide datatypes)
            * \param data                       Pointer to the original data
            * \param blockTimeWrite_Ms          How long the task calling this method should be put into a blocked state (max) until it is allowed to write to ONE of the queues (milliseconds)
            * \param blockTimeTakeSemaphore_Ms  How long the task calling this method should be put into a blocked state (max) until is is allowed to aquire ONE of the semaphores for the operation (milliseconds)
            * \param u32AdditionalInfo          32 Bit additional payload along with void pointer 'data'
            * \return                           True if it was successfully written to ALL tx connections. <br>
            *                                   False if at least one write operation failed
            * \note                             Supplied block-time's will be used for each write operation!
            * \note                             Databroadcast will proceed even if a write operation failed <br>
            *                                   Here you can pass a 32 bit variable along with the data pointer. <br>
            *                                   You can have the same functionality but with an additional 64 bit payload instead, by uncommenting FRTTRANSCEIVER_64BITADDITIONALDATA
            *                                   in FRTTransceiverSettings.h
            * \attention                        Do not use this feature when you have a lot of tx connections
            *                                   and it's particularly hard to get instant permission to write on at least one of them<br>
            *                                   However, if you still need to, at least keep  blockTimeWrite_Ms and blockTimeTakeSemaphore_Ms very low                   
            */
            bool databroadcast(uint8_t u8Datatype,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint32_t u32AdditionalInfo);
            #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
            /*! 
            * \brief                            Broadcasts data to all communication lines with a tx connection
            * \param u8Datatype                 Datatype of void pointer 'data'. (Create your own system wide datatypes)
            * \param data                       Pointer to the original data
            * \param blockTimeWrite_Ms          How long the task calling this method should be put into a blocked state (max) until it is allowed to write to ONE of the queues (milliseconds)
            * \param blockTimeTakeSemaphore_Ms  How long the task calling this method should be put into a blocked state (max) until is is allowed to aquire ONE of the semaphores for the operation (milliseconds)
            * \param u64AdditionalInfo          64 Bit additional payload along with void pointer 'data'
            * \return                           True if it was successfully written to ALL tx connections. <br>
            *                                   False if at least one write operation failed
            * \note                             Supplied block-time's will be used for each write operation!
            * \note                             Databroadcast will proceed even if a write operation failed <br>
            *                                   Here you can pass a 64 bit variable along with the data pointer. <br>
            *                                   You can have the same functionality but with an additional 32 bit payload instead, by uncommenting FRTTRANSCEIVER_32BITADDITIONALDATA
            *                                   in FRTTransceiverSettings.h
            * \attention                        Do not use this feature when you have a lot of tx connections
            *                                   and it's particularly hard to get instant permission to write on at least one of them<br>
            *                                   However, if you still need to, at least keep  blockTimeWrite_Ms and blockTimeTakeSemaphore_Ms very low                   
            */
            bool databroadcast(uint8_t u8Datatype,void * data,int blockTimeWrite_Ms,int blockTimeTakeSemaphore_Ms,uint64_t u64AdditionalData);
            #endif
            /*! 
            * \brief                            Reads from a selected queue (either normal queue or multi-sender-queue)
            * \param partner                    To select the queue to read from (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue           To select the multi-sender-queue to read from (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar          Signals whether to use partner parameter or multiSenderQueue parameter
            * \param blockTimeRead_Ms           How long the task calling this method should be put into a blocked state (max) until it is allowed to read from the queue (milliseconds)
            * \param blockTimeTakeSemaphore_Ms  How long the task calling this method should be put into a blocked state (max) until is is allowed to aquire the semaphore for the operation (milliseconds)
            * \return                           True if data read
            * \note                             Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                                   If partner is used, bUseTaskHandleVar is set to true.<br>
            *                                   If multiSenderQueue is used, bUseTaskHandleVar is set to false                         
            */
            bool readFromQueue(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,int blockTimeRead_Ms,int blockTimeTakeSemaphore_Ms);
            /*! 
            * \brief                            Flushes rx/tx queue                      
            * \param partner                    To select the queue to flush (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue           To select the multi-sender-queue to flush (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar          Signals whether to use partner parameter or multiSenderQueue parameter
            * \param blockTimeTakeSemaphore_Ms  How long the task calling this method should be put into a blocked state (max) until is is allowed to aquire the semaphore for the operation (milliseconds)
            * \param bTxQueue                   Signals whether to use the tx line of the connection or the rx line
            * \return                           True if queue flushed 
            * \note                             Here either FRTTTaskHandle or a eMultiSenderQueue enum is supplied. <br>
            *                                   If partner is used, bUseTaskHandleVar is set to true.<br>
            *                                   If multiSenderQueue is used, bUseTaskHandleVar is set to false                                                 
            */
            bool queueFlush(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,int blockTimeTakeSemaphore_Ms,bool bTxQueue);

            /*! 
            * \brief                        Deletes an entry in the internal rx buffer                      
            * \param partner                To select the specific internal rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter
            * \param u8PositionInBuffer     Position in rx buffer to clear
            * \return                       True if data "deleted"
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false                                               
            */
            bool delDatabuffForLine(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8PositionInBuffer);
            /*! 
            * \brief                        Flushes the rx buffer for a specific communication line                    
            * \param partner                To select the specific internal rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter
            * \return                       True if buffer flushed 
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false                                                 
            */
            bool delAllDatabuffForLine(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);

            /*! 
            * \brief                        Deletes oldest entry in the internal rx buffer (of a normal communication line)
            * \param partner                To select the specific internal rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \return                       True if oldest data deleted
            * \note                         Rx buffer : {  [[pos 0]oldest data][pos 1][pos 2].....[pos n-1][[pos n]newest data]  }                   
            */                                          
            bool delOldestDatabuffForLine(FRTTTaskHandle partner);
            /*! 
            * \brief                        Deletes oldest entry in the internal rx buffer of a multi-sender-queue
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \return                       True if oldest data deleted
            * \note                         Rx buffer : {  [[pos 0]oldest data][pos 1][pos 2].....[pos n-1][[pos n]newest data]  }                         
            */ 
            bool delOldestDatabuffForLine(eMultiSenderQueue multiSenderQueue);
            /*! 
            * \brief                        Deletes latest entry in the rx buffer for a specific communication line                  
            * \param partner                To select the specific internal rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter
            * \return                       True if newest data deleted
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false                                              
            */
            bool delNewestDatabuffForLine(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
            /*! 
            * \brief                        Returns the amount of messages on a selected queue
            * \param partner                To select the queue to check the amount of messages from (Used to select the right entry in FRTTCommunicationPartner)
            * \param bCheckTxQueue          Signals whether to check tx or rx queue
            * \return                       Amount of messages on the selected queue or -1                         
            */
            int messagesOnQueue(FRTTTaskHandle partner, bool bCheckTxQueue);
            /*! 
            * \brief                        Returns the amount of messages on a selected multi-sender-queue (multiple tasks write on the tx line)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue to check the amount of messages from (Used to select the right entry in FRTTCommunicationPartner)
            * \return                       Amount of messages on the selected queue or -1                           
            */
            int messagesOnQueue(eMultiSenderQueue multiSenderQueue);

            /*! 
            * \brief                        Checks if buffered rx data available                  
            * \param partner                To select the specific internal rx buffer to check for data (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer to check for data (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter
            * \return                       True if buffered data available
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false                                                
            */
            bool hasDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
            /*! 
            * \brief                        Returns the amount of buffered data for a specific communication line                  
            * \param partner                To select the specific internal rx buffer to check for the amount of buffered data (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer to check for the amount of buffered data (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter
            * \return                       Amount of buffered data or -1
            * \note                         Here either FRTTTaskHandle or a eMultiSenderQueue enumerator is supplied. <br>
            *                               If partner is used, bUseTaskHandleVar is set to true.<br>
            *                               If multiSenderQueue is used, bUseTaskHandleVar is set to false                                                
            */
            int bufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
            /*! 
            * \brief                        Returns the amount of buffered data for a all communication lines combined                
            * \return                       Amount of buffered data or -1 if _bHasValidStruct is true                                             
            */
            int bufferedDataInAllBuffers();
        
            /*! 
            * \brief                        Checks if datatype found in rx buffer for a specific communication line
            * \param partner                To select the specific internal rx buffer to check for the datatype (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer to check for the datatype (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter
            * \param u8Datatype             Datatype to look for (Create your own system wide datatypes)
            * \return                       Amount of datatype occurences or -1                        
            */
            int isDatatypeInBuffer(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8Datatype);

            /*! 
            * \brief                        Returns the newest buffered data for a specific communication line
            * \param partner                To select the specific internal rx buffer from which the data should be taken (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer from which the data should be taken (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter 
            * \return                       Const pointer to the buffer position or NULL
            * \note                         Rx buffer : {  [[pos 0]oldest data][pos 1][pos 2].....[pos n-1][[pos n]newest data]  }                          
            */
            const FRTTTempDataContainer * getNewestBufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
            /*! 
            * \brief                        Returns the oldest buffered data for a specific communication line
            * \param partner                To select the specific internal rx buffer from which the data should be taken (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer from which the data should be taken (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter 
            * \return                       Const pointer to the buffer position or NULL
            * \note                         Rx buffer : {  [[pos 0]oldest data][pos 1][pos 2].....[pos n-1][[pos n]newest data]  }                          
            */
            const FRTTTempDataContainer * getOldestBufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar);
            /*! 
            * \brief                        Returns a selected part of the buffered data for a specific communication line
            * \param partner                To select the specific internal rx buffer from which the data should be taken (Used to select the right entry in FRTTCommunicationPartner)
            * \param multiSenderQueue       To select the specific internal multi-sender-queue rx buffer from which the data should be taken (Used to select the right entry in FRTTCommunicationPartner)
            * \param bUseTaskHandleVar      Signals whether to use partner parameter or multiSenderQueue parameter 
            * \param u8PositionInBuffer     Position from which the data should be taken
            * \return                       Const pointer to the buffer position or NULL
            * \note                         Rx buffer : {  [[pos 0]oldest data][pos 1][pos 2].....[pos n-1][[pos n]newest data]  }                          
            */
            const FRTTTempDataContainer * getBufferedDataFrom(FRTTTaskHandle partner,eMultiSenderQueue multiSenderQueue,bool bUseTaskHandleVar,uint8_t u8PositionInBuffer);
			/*! 
            * \brief                            Sets the partner tasks notification state to pending (if not already pending!!) and increments its notification value (adds 1 to the value).
            * \param partner                    Partner task to notify
            * \return						    Will always return true unless you provided wrong parameters 
            *                                   Will return false if any of the situations mentioned in 'note' below happened 
            * \note                             Even if the receving task has a notification pending, this method will always increment the partner tasks notification value!                    
            * \note                             Internally the library only checks: - Partner has been added to the list of communications<br>
            *                                                                       - Partner is not a multiSenderQueue || partner is not nullptr<br>
            *                                   There is currently no solution available to check if a FRTTTaskhandle is valid so the correct execution of this method is in your hands<br>
            *                                   An invalid FRTTTaskhandle will result in a FreeRTOS crash!
            */
            bool NotifyBasic(FRTTTaskHandle partner);
            /*! 
            * \brief                            Extended version of FRTT:FRTTransceiver::NotifyBasic()
            * \details                          In this version of the notify funcionality you can do: 
            *                                       - Increment partner tasks notification value by 1 (equivalent to FRTT:FRTTransceiver::NotifyBasic())
            *                                       - Set single bits in the partner tasks notification value (---> lightweight eventgroup possible)
            *                                       - Update partner tasks notification value (only if partner task has no notification pending) (similar to a queue with length 1)
            *                                       - Update parnter tasks notification value (even if partner task has notification pending) (similar to queueOverwrite())
            * \param partner                    Partner task to notify         
            * \param action                     Ways to update the notification value : [e_NoAction, e_SetBits, e_Increment,e_SetValueWithOverwrite,e_SetValueWithoutOverwrite]<br>
            *                                   Check their description in FRTTransceiverNotify.h
            * \param u32NotificationMask        Notification value/mask. If its gonna be used depends on the action provided.           
            * \return						    Almost always true, except when action == eFRTTNotifyActions::eSetValueWithoutOverwrite and the partner task had a notification pending
            * \note                                                 
            */
            bool NotifyExtended(FRTTTaskHandle partner,eFRTTNotifyActions action,uint32_t u32NotificationMask);
			/*! 
            * \brief                        	Checks if the owner of this object has a notification pending
            * \param 	action                	Set to FRTT:eNotifyAction::eCLEARCOUNTONEXIT to tell FreeRTOS to set the notification value to zero at the end
            *                                   Set to FRTT:eNotifyAction::eCLEARCOUNTONEXITNOT to tell FreeRTOS to simply decrement the notification value at the end
            * \return							*This object. Immediately call FRTTransceiver::getNotificationVal() or FRTTransceiveR:hasNotification() to infos.        
            * \attention                        The final notificaiton value put into the internal buffer can be 0. That would mean that no task notified us and FRTTransceiveR:hasNotification() will therefore return false<br>
            *                                   The internal FreeRTOS method used in this basic version only checks for a notification value > 0
            *                                                        
            */
            FRTTransceiver & NotifyReceiveBasic(eFRTTNotifyActions action,int blockTimeReceive_Ms);
            /*! 
            * \brief                        	Extended version of FRTT::FRTTransceiver::NofityReceiveBasic()
            * \details                          In this version of the notify-receive functionality you can do:
            *                                       - Clear bits (RIGHT ONENTRY) of the notification value of the owner task of this object (will only be cleared if no notification pending)
            *                                       - Clear bits (AFTER xTaskNotifyWait()) of the notification value of the owner task of this object <br>
            *                                         (will only be cleared if notification received / notification was already pending)
            * \param    u32ClearOnEntryMask     Same bits as in this mask will be cleared (ONENTRY, before 'receive') in the notification value of the owner of this task (will only be cleared if no notification pending)<br>
            *                                   Example: u32ClearOnEntryMask == 0x03, notificationBefore == 0x0011 ---> notificationValUpdated == 0x0000
            * \param    u32ClearOnExitMask      Same bits as in this mask will be cleared (ONEXIT, after 'receive') in the notification value of the owner of this task (only if notification was received)
            *                                   Example: u32ClearOnEntryMask == 0x03, notificationBefore == 0x0011 ---> notificationValUpdated == 0x0000                                    
            * \return							*This object. Immediately call FRTTransceiver::getNotificationVal() or FRTTranceiver::hasNotification() to get infos      
            * \note                             This method will also signal that a notification was received when the notification value == 0!                     
            */
            FRTTransceiver & NotifyReceiveExtended(uint32_t u32ClearOnEntryMask,uint32_t u32ClearOnExitMask,int blockTimeReceive_Ms);
            /*! 
            * \brief                            Returns the last notification value received	           
            * \return					        32 Bit notification value                             
            * \note                             A call to FRTTransceiver::NotifyReceiveBasic() or FRTTransceiver::NotifiyReceiveExtended() will update this value
            * \note                             Return value -> Greater than zero: notification received. Zero:  Nothing received. 
            */
            uint32_t getNotificationVal();
            /*! 
            * \brief                            Sets the internal notification variable to zero	           
            * \return					        void                            
            */
            void clearNotificationVal();
            /*! 
            * \brief                            Returns whether we had receveid a notification with the last call to any of the 'NotifyReceive' methods.      
            * \return                           True or false                            
            */
            bool hasNotification();
            /*! 
            * \brief                        To to add an allocator callback function
            * \details                      This method is used to add an allocator callback function to the library.
            *                               When the library reads data off the queue, it will do so by copying its content into a FRTTDataContainerOnQueue structure.<br>
            *                               After that, the library aquires the next free space in the internal rx buffer (FRTTTempDataContainer) and places a call to the allocator callback<br>
            *                               Now the callback function will be called, in which the user of the library can just copy everything (pointer to the original data of communication partner) 
            *                               over OR allocate memory (e.g malloc) and then copy data into own variables.
            * \param fP                     Pointer to a function which takes FRTTDataContainerOnQueue and FRTTTempDataContainer as an argument.
            * \return                       void
            * \attention                    Later versions of the library will support some sort of memory pool to allocate/free data through the callbacks
            */ 
            void addDataAllocateCallback(void(*fP)(const FRTTDataContainerOnQueue &, FRTTTempDataContainer &));
            /*! 
            * \brief                        To to add a de-allocator callback function
            * \details                      This method is used to add a de-allocator callback function to the library.
            *                               The user of the library should basically reverse the steps made in FRTTransceiver::addDataAllocateCallback() <br>
            * \param fP                     Pointer to a function which takes FRTTDataContainerOnQueue and FRTTTempDataContainer as an argument.
            * \return                       void
            * \attention                    Later versions of the library will support some sort of memory pool to allocate/free data through the callbacks
            */
            void addDataFreeCallback(void (*fP)(FRTTTempDataContainer &));
            

            #if defined(FRTTRANSCEIVER_ANALYTICS_ENABLE)
            /*! 
            * \brief                        Prints a good summary of all communication lines (TURN ON by uncommenting #FRTTRANSCEIVER_ANALYTICS_ENABLE in FRTTransceiverSettings.h)
            * \return                       void                           
            */
            void printCommunicationsSummary();
            #endif
    };
}
#endif