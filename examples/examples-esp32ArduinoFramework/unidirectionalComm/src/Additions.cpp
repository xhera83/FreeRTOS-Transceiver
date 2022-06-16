#include "Additions.h"

void SENDER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

	#if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_SENDER,1);
	#else
	FRTTCommunicationPartner partner[1];
	FRTTransceiver comm(TASK_SENDER,&partner[0],1);
	#endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_RECEIVER,nullptr,0,nullptr,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,"RECEIVER");

    if(retVal)
    {
        printf("\tConnected to communication line with the RECEIVER_TASK\n");
    }


    char *messages[] = {"First message","Second message","Third message","Fourth message","Fifth message"};
    uint32_t u32AdditionalData = 0;
    
    for(uint8_t u8I = 0;u8I < 5 ;u8I++)
    {
        bool res = comm.writeToQueue(TASK_RECEIVER,eMESSAGE,&messages[u8I][0],FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,++u32AdditionalData);

        if(res)
        {
            printf("\tData sent...\n");
        }
        else 
        {
            printf("\tCouldnt send the data...\n");
        }
        vTaskDelay(pdMS_TO_TICKS(750));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
    //comm.printCommunicationsSummary();
    #endif
    vTaskDelete(nullptr);
}

void RECEIVER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

	#if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_RECEIVER,1);
	#else
	FRTTCommunicationPartner partner[1];
	FRTTransceiver comm(TASK_RECEIVER,&partner[0],1);
	#endif

    
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,1,SEMAPHORE1,nullptr,0,nullptr,"SENDER");

    if(retVal)
    {
        printf("\tConnected to communication line with the SENDER_TASK\n");
    }
    
    uint32_t u32AdditionalData = 0;
    
    for(;;)
    {
        if(comm.messagesOnQueue(TASK_SENDER,false) > 0)
        {
            bool res = comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);

            if(res)
            {
                const FRTTTempDataContainer * t = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0);
                
                if(t != nullptr)
                {
                    printf("\tReceived data from : %p\n",t->senderAddress);
                    
                    if(t->u8DataType == eMESSAGE)
                    {
                        printf("\tDATA [Message]: %s\n",(char *)t->data);
                        u32AdditionalData = t->u32AdditionalData;
                        printf("\tAdditional data: %d\n",u32AdditionalData);
                    }
                    comm.delAllDatabuffForLine(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
                }
            }
        }
        if(u32AdditionalData == STOP_COMM_AT)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
    comm.printCommunicationsSummary();
    #endif
    vTaskDelete(nullptr);
}

void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer){

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and data a destroyer callback function.
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
     *                    ---> Use malloc/new (not recommended)
     *                    ---> Later implementations might provide some sort of internal memory pool implementation
     */ 

    internalBuffer.u8DataType = origingalContainer_onQueue.u8DataType;
    internalBuffer.u32AdditionalData = origingalContainer_onQueue.u32AdditionalData;
    internalBuffer.senderAddress = origingalContainer_onQueue.senderAddress;
    internalBuffer.data = origingalContainer_onQueue.data;
}

void dataDestroyer(FRTTTempDataContainer & internalBuffer) {

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and data destroyer callback function.
     *      
     *      To do:
     *          
     *          (1): 
     *               - Reverse the actions made in the allocator callback function (if malloc()/new was used then ---> free()/delete) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = nullptr;
    internalBuffer.data = nullptr;
}