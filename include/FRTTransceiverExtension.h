#ifndef FRTTRANSCEIVEREXTENSION_H
#define FRTTRANSCEIVEREXTENSION_H
/*!
 * \file       FRTTransceiverExtension.h
 * \brief      Extends the library to important structures, defines and variable types
 * \author     Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \copyright  Copyright 2022 Xhemail Ramabaja
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
    /*! \brief  Often used as the return value of FreeRTOS functions */
    typedef UBaseType_t FRTTransceiver_BaseType;
    /*! \brief  Holds the address to a queue*/
    typedef QueueHandle_t FRTTransceiver_QueueHandle;
    /*! \brief  Holds the address to a task */
    typedef TaskHandle_t FRTTransceiver_TaskHandle;
    /*! \brief  Holds the address to a semaphore */
    typedef SemaphoreHandle_t FRTTransceiver_SemaphoreHandle;

    /*! \brief Structure holding a data package that will be put on a queue */
    struct FRTTransceiver_DataContainerOnQueue
    {
        FRTTransceiver_TaskHandle senderAddress;              /*!< Address of the task which is the source of the data package */
        void * data;                                          /*!< Pointer to the original data that is being send over the queue*/
        uint8_t u8DataType;                                   /*!< Datatype of the data that the void pointer points to*/
        #if defined(FRTTRANSCEIVER_32BITADDITIONALDATA)
        uint32_t u32AdditionalData;                           /*!< 32 Bit additional data. Can be used for different data (TURN ON/OFF in FRTTransceiverSettings.h)*/
        #elif defined(FRTTRANSCEIVER_64BITADDITIONALDATA)
        uint64_t u64AdditionalData;                           /*!< 64 Bit additional data. Can be used for different data (TURN ON/OFF in FRTTransceiverSettings.h)*/
        #endif
    };

    /*! \brief Will be used as rx buffer*/
    typedef FRTTransceiver_DataContainerOnQueue FRTTransceiver_TempDataContainer;

    /*! \brief Structure holding all important data regarding a communication line*/
    struct FRTTransceiver_CommunicationPartner
    {
        FRTTransceiver_TaskHandle commPartner = NULL;                                                      /*!< Holds the address of the partner task or NULL in the case of a multi-sender-queue*/
        string partnersName;                                                                               /*!< Holds the self set name of the partner task or FRTTRANSCEIVER_DEFAULTPARTNERNAMExxxxxxxx*/

        bool bReadOnlyCommunication = false;                                                               /*!< Is set to true if the communication line is meant for a readonly multi-sender-queue. RX*/

        FRTTransceiver_SemaphoreHandle semaphoreRxQueue = NULL;                                            /*!< Semaphore to get access to the rx queue (partner sets it as the semaphore for the tx queue)*/
        FRTTransceiver_SemaphoreHandle semaphoreTxQueue = NULL;                                            /*!< Semaphore to get access to the tx queue (partner sets it as the semaphore for the rx queue)*/

        uint8_t u8TxQueueLength = 0;                                                                       /*!< Length of the tx queue (> 0)*/
        uint8_t u8RxQueueLength = 0;                                                                       /*!< Length of the rx queue (> 0)*/

        struct FRTTransceiver_DataContainerOnQueue txLineContainer[FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE];  /*!< Array holding data packages that are currently lying / have previously lain on the queue*/
        struct FRTTransceiver_DataContainerOnQueue rxLineContainer;                                        /*!< Intermediate container between queue and internal buffers */

        FRTTransceiver_QueueHandle rxQueue = NULL;                                                         /*!< Holds the address of the rx queue*/
        FRTTransceiver_QueueHandle txQueue = NULL;                                                         /*!< Holds the address of the tx queue*/

        FRTTransceiver_TempDataContainer tempContainer[FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE];              /*!< Internal buffers. Data read from the queue end up in this buffer */

        #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE                                                             
        int dataPackagesReceived = 0;                                                                      /*!< Hold the amount of data packages received. (TURN ON/OFF in FRTTransceiverSettings.h)*/
        int dataPackagesSent = 0;                                                                          /*!< Hold the amount of data packages sent. (TURN ON/OFF in FRTTransceiverSettings.h)*/
        #endif

        bool hasBufferedData = false;                                                                      /*!< Signals whether we have buffered data for that specific communication line*/
        bool rxBufferFull = false;                                                                         /*!< Signals whether the internal buffer is full. Only for internal use. */
        int8_t i8CurrTempcontainerPos = -1;                                                                /*!< Stores the current position in the internal buffer in which to write next */
    };

    /*! \brief Pointer to the data allocator callback a user needs to supply in order to use this library */
    using fP_dataAllocateCallback =  void (*)(const FRTTransceiver_DataContainerOnQueue &,FRTTransceiver_TempDataContainer &);
    /*! \brief Pointer to the data de-allocator callback a user needs to supply in order to use this library */
    using fP_dataFreeCallback = void (*)(FRTTransceiver_TempDataContainer &);
}
#endif