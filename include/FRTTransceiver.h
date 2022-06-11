#ifndef FRTTRANSCEIVER_H
#define FRTTRANSCEIVER_H

/*!
 * \file        FRTTransceiver.h
 * \brief       Class definition headerfile
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version     v1.1.0
 * \copyright   Copyright 2022 Xhemail Ramabaja
 */

#include "FRTTransceiverExtension.h"
#include <string>

using namespace std;

namespace FRTT {
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
            bool _bDelete = false;                                                  /*!< Signals whether delete [] is needed in the destructor                          */
            bool _bHasValidStruct = false;                                          /*!< Signals whether ::_structCommPartners is NOT nullptr || _u8MaxPartners != 0    */

            #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
            int _broadcastCount = 0;                                                /*!< Amount of broadcasts made. Important for FRTTransceiver::printCommunicationsSummary() */
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
            * \param ownerAddress           Address of the task owning this object (can be null, other tasks wont know who the source of messages is)
            * \param u8MaxPartners          Max possible connections            
            * \attention                    If u8MaxPartners is 0, then the constructor will internally increment it to 1 (min 1 partner)!             
            */
            FRTTransceiver(FRTTTaskHandle ownerAddress,uint8_t u8MaxPartners);
            /*! 
            * \brief                        FRTTransceiver Constructor (2)
            * \details                      Receives a pre-declared array of FRTTCommunicationPartner
            * \param ownerAddress           Address of the task owning this object (can be null, other tasks wont know who the source of messages is)
            * \param u8MaxPartners          Max possible connections
            * \param commStructs            Pointer to the pre-declared array of FRTTCommunicationPartner.
            * \attention                    If commStructs is a nullptr OR u8MaxPartners is 0, then all class methods wont work<br>
            *                               Supplying the wrong u8MaxPartners to the commStruct will result in undefined library behaviour (so make sure array size and u8MaxPartners match!)                  
            */
            FRTTransceiver(FRTTTaskHandle ownerAddress,FRTTCommunicationPartner * commStructs,uint8_t u8MaxPartners):    _ownerAddress(ownerAddress),
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