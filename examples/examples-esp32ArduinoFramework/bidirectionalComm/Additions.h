/*!
 * \file        Additions.h
 * \brief       Additional data for the examples         
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include "FRTTransceiver.h"

using namespace FRTT;

/* datatypes recognized throughout the example */
typedef enum
{
   eCOMMAND = 0,
}eDataTypes;


FRTTTaskHandle TASK_SENDER;
FRTTTaskHandle TASK_RECEIVER;

FRTTQueueHandle QUEUE_TO_RECEIVER;
FRTTQueueHandle QUEUE_FROM_RECEIVER;

FRTTSemaphoreHandle SEMAPHORE1;
FRTTSemaphoreHandle SEMAPHORE2;

#define QUEUELENGTH 1

/* COMMANDS BETWEEN TASK_RECEIVER TO TASK_SENDER */
#define COMMAND_STOP             (0u)
#define COMMAND_SEND             (1u)
#define COMMAND_TURNLEDON        (2u)
#define COMMAND_TURNLEDOFF       (3u)


void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer){

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and data destroyer callback function.
     *      
     *      To do:
     *          
     *          (1): 
     *               - Copy u8Datatype variable
     *               - Copy additionalData variable
     *               - Copy senderAdress variable
     *          
     *          (2): 
     *               - Provide some sort of way to copy the main data over:
     *                    ---> Just copy the pointer over
     *                    ---> Use malloc (not recommended)
     *                    ---> Later implementations might provide some sort of internal memory pool implementation
     */ 

    internalBuffer.u8DataType = origingalContainer_onQueue.u8DataType;
    internalBuffer.u32AdditionalData = origingalContainer_onQueue.u32AdditionalData;
    internalBuffer.senderAddress = origingalContainer_onQueue.senderAddress;
    internalBuffer.data = origingalContainer_onQueue.data;

    switch (origingalContainer_onQueue.u8DataType)
    {
        case eCOMMAND:
            //  internalBuffer.data = (int *)malloc(sizeof(uint8_t));
            //  *((uint8_t *)internalBuffer.data) = *((uint8_t *)origingalContainer_onQueue.data);
            break;
        default:
            break;
    }
}

void dataDestroyer(FRTTTempDataContainer & internalBuffer) {

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and data destroyer callback function.
     *      
     *      To do:
     *          
     *          (1): 
     *               - Reverse the actions made in the allocator callback function (malloc() ---> free()) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = NULL;
  
    switch(internalBuffer.u8DataType)
    {
        case eCOMMAND:
            //  free((uint8_t *)internalBuffer.data); 
            internalBuffer.data = NULL;
            break;
        default:
            break;
    }
}