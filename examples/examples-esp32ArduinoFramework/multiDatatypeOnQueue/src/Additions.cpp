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

    bool retVal = comm.addCommPartner(TASK_RECEIVER,nullptr,0,nullptr,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,"RECEIVER");

    if(retVal)
    {
        printf("\tConnected to communication line with the RECEIVER_TASK\n");
    }


    char * message = "This is a message produced by SENDER";
    struct TestDType testData;
    testData.var1 = 1;
    testData.var2 = 1;
    int a = 1;

    uint32_t u32AdditionalData = 0;
    
    for(uint8_t u8I = 0;u8I < 7;u8I++)
    {   
        bool res1 = comm.writeToQueue(TASK_RECEIVER,eSTRUCT,&testData,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,++u32AdditionalData);
        bool res2 = comm.writeToQueue(TASK_RECEIVER,eMESSAGE,&message,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,++u32AdditionalData);
        bool res3 = comm.writeToQueue(TASK_RECEIVER,eINT,&a,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,++u32AdditionalData);

        if(res1 && res2 && res3)
        {
            printf("\tAll data transmitted!\n");
        }

        vTaskDelay(pdMS_TO_TICKS(700));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
    //comm.printCommunicationsSummary();
    #endif

    comm.~FRTTransceiver();
    vTaskDelete(nullptr);
}

void RECEIVER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

	#if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_RECEIVER,1);         
    #else
	FRTTCommunicationPartner commStruct[1];
    FRTTransceiver comm(TASK_RECEIVER,&commStruct[0],1);
    #endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,nullptr,0,nullptr,"SENDER");

    if(retVal)
    {
        printf("\tConnected to communication line with the SENDER_TASK\n");
    }
    
    uint8_t u8I = 0;
    for(;;)
    {
        if(comm.messagesOnQueue(TASK_SENDER,false) == 3)
        {
            bool res1 = comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);
            bool res2 = comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);
            bool res3 = comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);

            if(res1 && res2 && res3)
            {
                printf("\tAll data read into internal buffers\n");
                const FRTTTempDataContainer * t1 = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0);
                const FRTTTempDataContainer * t2 = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,1);
                const FRTTTempDataContainer * t3 = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,2);
                
                if(t1 && t2 && t3)
                {
                    printf("\tPackage sender addresses (Package[1]:%p, Package[2]:%p, Package[3]:%p)\n",t1->senderAddress,t2->senderAddress,t3->senderAddress);
                    printf("\tAdditional data for each received package: Package[1]: %d Package[2]: %d Package[3]: %d\n",t1->u32AdditionalData,t2->u32AdditionalData,
                                                                                                                                            t3->u32AdditionalData);
                    printf("\tOccurence of eMESSAGE in buffer: %d\n",comm.isDatatypeInBuffer(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,eMESSAGE));
                    printf("\tOccurence of eSTRUCT in buffer: %d\n",comm.isDatatypeInBuffer(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,eSTRUCT));
                    printf("\tOccurence of eINT in buffer: %d\n",comm.isDatatypeInBuffer(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,eINT));
                    u8I++;
                }

                comm.delAllDatabuffForLine(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
            }
        }
        if(u8I == 7)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(60));
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
     *               - Reverse the actions made in the allocator callback function (if malloc()/new were used ---> free()/delete) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = nullptr;
    internalBuffer.data = nullptr;
}