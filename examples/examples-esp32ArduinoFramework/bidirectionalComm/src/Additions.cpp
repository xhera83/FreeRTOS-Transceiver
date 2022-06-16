#include "Additions.h"

void SENDER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

	#if (DYNAMIC_STRUCTARRAY == 1)
	FRTTransceiver comm(TASK_SENDER,1);
	#else
    FRTTCommunicationPartner commStruct[1];
    FRTTransceiver comm(TASK_SENDER,&commStruct[0],1);
	#endif


    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_RECEIVER,QUEUE_FROM_RECEIVER,QUEUELENGTH,SEMAPHORE2,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,"RECEIVER");

    if(retVal)
    {
        printf("\tConnected to communication line RECEIVER \n");
    }
    
    uint8_t u8COMMANDS[10] = {COMMAND_TURNLEDOFF,COMMAND_TURNLEDON,COMMAND_TURNLEDON,COMMAND_TURNLEDON,COMMAND_TURNLEDOFF,
                              COMMAND_TURNLEDOFF,COMMAND_TURNLEDOFF,COMMAND_TURNLEDOFF,COMMAND_TURNLEDOFF,COMMAND_TURNLEDOFF};
    uint8_t u8PosInBuff = 0;
    uint32_t u32AdditionalData = 0; /* Not used */

    for(;;)
    {
        if(comm.messagesOnQueue(TASK_RECEIVER,false) > 0)
        {
            bool retVal = comm.readFromQueue(TASK_RECEIVER,eMultiSenderQueue::eNOMULTIQSELECTED,true,100,100);

            if(retVal && comm.hasDataFrom(TASK_RECEIVER,eMultiSenderQueue::eNOMULTIQSELECTED,true))
            {
                const FRTTTempDataContainer * t = comm.getNewestBufferedDataFrom(TASK_RECEIVER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
                
                if(t != nullptr)
                {
                    uint8_t u8COMMAND = *((uint8_t *)t->data);

                    comm.delDatabuffForLine(TASK_RECEIVER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0);

                    if(u8COMMAND  == COMMAND_SEND)
                    {
                        printf("\tThe receiver-task (%p) asked for instructions...\n",t->senderAddress);
                        comm.writeToQueue(TASK_RECEIVER,eCOMMAND,&u8COMMANDS[u8PosInBuff],100,100,u32AdditionalData);
                        u8PosInBuff++;
                    }
                    else if(u8COMMAND == COMMAND_STOP)
                    {
                        printf("\tThe receiver-task (%p) asked to stop the communication...\n",t->senderAddress);
                        comm.~FRTTransceiver();
                        break;
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(750));
    }

    #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
    //comm.printCommunicationsSummary();
    #endif

    vTaskDelete(nullptr);
}


void RECEIVER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if printf()'s happen */

	#if (DYNAMIC_STRUCTARRAY == 1)
	FRTTransceiver comm(TASK_RECEIVER,1);
	#else
    FRTTCommunicationPartner commStruct[1];
    FRTTransceiver comm(TASK_RECEIVER,&commStruct[0],1);
	#endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,QUEUE_FROM_RECEIVER,QUEUELENGTH,SEMAPHORE2,"SENDER");

    if(retVal)
    {
        printf("\tConnected to communication line SENDER");
    }

    uint8_t u8COMMAND = COMMAND_SEND;
    uint8_t u8Counter = 0;
    uint32_t u32AdditionalData = 0; /* In this example not used */

    comm.writeToQueue(TASK_SENDER,eCOMMAND,&u8COMMAND,100,100,u32AdditionalData);

    for(;;)
    {
        if(comm.messagesOnQueue(TASK_SENDER,false) > 0)
        {
            bool retVal = comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,100,100);

            if(retVal && comm.hasDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true))
            {
                const FRTTTempDataContainer * t = comm.getNewestBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
                
                if(t != nullptr)
                {
                    printf("\tData received by %p\n",t->senderAddress);
                    u8COMMAND = *((uint8_t *)t->data);

                    switch(u8COMMAND) 
                    {
                        case COMMAND_TURNLEDON:
                            printf("\tCommand received: turn led on\n");
                            break;
                        case COMMAND_TURNLEDOFF:
                            printf("\tCommand received: turn led off\n");
                            break;
                    }
                    u8Counter++;
                    comm.delAllDatabuffForLine(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
                    if(u8Counter == 9)
                    {
                        u8COMMAND = COMMAND_STOP;
                        comm.writeToQueue(TASK_SENDER,eCOMMAND,&u8COMMAND,100,100,u32AdditionalData);
                        break;
                    }
                    else
                    {
                        u8COMMAND = COMMAND_SEND;
                        comm.writeToQueue(TASK_SENDER,eCOMMAND,&u8COMMAND,100,100,u32AdditionalData);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(750));
    }

    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
    //comm.printCommunicationsSummary();
    #endif 

    vTaskDelete(nullptr);
}


void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer){

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and a data destroyer callback function.
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
     *      data allocator and a data destroyer callback function.
     *      
     *      To do:
     *          
     *          (1): 
     *               - Reverse the actions made in the allocator callback function (when malloc() used ---> free()) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = nullptr;
  
    switch(internalBuffer.u8DataType)
    {
        case eCOMMAND:
            //  free((uint8_t *)internalBuffer.data); 
            internalBuffer.data = nullptr;
            break;
        default:
            break;
    }
}