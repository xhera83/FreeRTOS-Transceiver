#ifndef FRTTRANSCEIVEREXTENSION_H
#define FRTTRANSCEIVEREXTENSION_H
/*!
 * \file        FRTTransceiverExtension.h
 * \brief       Extends the library to important structures, defines and variable types
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version     v1.1.0
 * \copyright   Copyright 2022 Xhemail Ramabaja
 *
 */
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "FRTTransceiverSettings.h"
#include <string>

using namespace std;

/*! \brief Defines the maximum time a task can wait for something to become available */
#define FRTTRANSCEIVER_WAITMAX (-1)

namespace FRTT {
    /*! \brief  Often used as the return value of a FreeRTOS functions */
    using FRTTBaseType = UBaseType_t;
    /*! \brief  Holds the address to a queue*/
    using FRTTQueueHandle = QueueHandle_t;
    /*! \brief  Holds the address to a task */
    using FRTTTaskHandle = TaskHandle_t;
    /*! \brief  Holds the address to a semaphore */
    using FRTTSemaphoreHandle = SemaphoreHandle_t;
    /*! \brief Describes a function which returns void and takes a void pointer */
    using FRTTTaskFunction = TaskFunction_t;

    /*! \brief Structure holding a data package that will be put on a queue */
    struct FRTTDataContainerOnQueue
    {
        FRTTTaskHandle senderAddress = (FRTTTaskHandle)0x0;                             /*!< Address of the task which is the source of the data package        */
        void * data = nullptr;                                                          /*!< Pointer to the original data that is being send over the queue     */
        uint8_t u8DataType = 0;                                                         /*!< Datatype of the data that the void pointer points to               */
        #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
        uint32_t u32AdditionalData = 0;                                                 /*!< 32 Bit additional data. Can be used for different data (TURN ON/OFF in FRTTransceiverSettings.h)   */
        #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
        uint64_t u64AdditionalData = 0;                                                 /*!< 64 Bit additional data. Can be used for different data (TURN ON/OFF in FRTTransceiverSettings.h)   */
        #endif
    };

    /*! \brief Will be used as rx buffer*/
    using FRTTTempDataContainer = FRTTDataContainerOnQueue;

    /*! \brief Structure holding all important data regarding a communication line*/
    struct FRTTCommunicationPartner
    {
        FRTTTaskHandle commPartner = nullptr;                                                   /*!< Holds the address of the partner task or NULL in the case of a multi-sender-queue              */
        string partnersName;                                                                    /*!< Holds the self set name of the partner task or FRTTRANSCEIVER_DEFAULTPARTNERNAMExxxxxxxx       */

        bool bReadOnlyCommunication = false;                                                    /*!< Is set to true if the communication line is meant for a readonly multi-sender-queue. RX        */

        FRTTSemaphoreHandle semaphoreRxQueue = nullptr;                                         /*!< Semaphore to get access to the rx queue (partner sets it as the semaphore for the tx queue)    */
        FRTTSemaphoreHandle semaphoreTxQueue = nullptr;                                         /*!< Semaphore to get access to the tx queue (partner sets it as the semaphore for the rx queue)    */

        uint8_t u8TxQueueLength = 0;                                                            /*!< Length of the tx queue (> 0)*/
        uint8_t u8RxQueueLength = 0;                                                            /*!< Length of the rx queue (> 0)*/

        struct FRTTDataContainerOnQueue txLineContainer[FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE];  /*!< Array holding data packages that are currently lying / have previously lain on the queue       */
        struct FRTTDataContainerOnQueue rxLineContainer;                                        /*!< Intermediate container between queue and internal buffers                                      */

        FRTTQueueHandle rxQueue = nullptr;                                                      /*!< Holds the address of the rx queue  */
        FRTTQueueHandle txQueue = nullptr;                                                      /*!< Holds the address of the tx queue  */

        FRTTTempDataContainer tempContainer[FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE];              /*!< Internal buffers. Data read from the queue end up in this buffer */

        #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE                                                             
        int dataPackagesReceived = 0;                                                           /*!< Hold the amount of data packages received. (TURN ON/OFF in FRTTransceiverSettings.h)   */
        int dataPackagesSent = 0;                                                               /*!< Hold the amount of data packages sent. (TURN ON/OFF in FRTTransceiverSettings.h)       */
        #endif

        bool bHasBufferedData = false;                                                          /*!< Signals whether we have buffered data for that specific communication line     */
        bool bRxBufferFull = false;                                                             /*!< Signals whether the internal buffer is full. Only for internal use.            */
        int8_t i8CurrTempcontainerPos = -1;                                                     /*!< Stores the current position in the internal buffer in which to write next      */
    };

    /*! \brief Pointer to the data allocator callback a user needs to supply in order to use this library       */
    using fP_dataAllocateCallback =  void (*)(const FRTTDataContainerOnQueue &,FRTTTempDataContainer &);
    /*! \brief Pointer to the data de-allocator callback a user needs to supply in order to use this library    */
    using fP_dataFreeCallback = void (*)(FRTTTempDataContainer &);
}
#endif