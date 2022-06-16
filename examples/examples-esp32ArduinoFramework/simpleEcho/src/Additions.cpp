#include "Additions.h"


void ECHO(void *)
{
    while(TASK_ECHO == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_ECHO,1);
    #else
	FRTTCommunicationPartner partners[1];
	FRTTransceiver comm(TASK_ECHO,&partners[0],1);
    #endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_ECHO,ECHO_QUEUE,QUEUELENGTH,SEMAPHORE1,ECHO_QUEUE,QUEUELENGTH,SEMAPHORE1,"ECHO");

    if(retVal)
    {
        printf("\tConnected to myself. Echo without any delay incoming. Summary afterwards\n");
        vTaskDelay(pdMS_TO_TICKS(2500));
    }
    
    int a = 0;

    for(int i = 0;i < 1000;i++)
    {
        if(comm.messagesOnQueue(TASK_ECHO,false) > 0)
        {
            bool res = comm.readFromQueue(TASK_ECHO,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,0);
            comm.delAllDatabuffForLine(TASK_ECHO,eMultiSenderQueue::eNOMULTIQSELECTED,true);
        }
        else
        {
            bool res = comm.writeToQueue(TASK_ECHO,eINT,&a,0,0,1000);
        }
    }

    #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
    comm.printCommunicationsSummary();
    #endif
    
    vTaskDelete(nullptr);
}

void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer){

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and d data destroyer callback function.
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
     *               - Reverse the actions made in the allocator callback function (if malloc()/new was used then  ---> free()/delete) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = nullptr;
    internalBuffer.data = nullptr;
}