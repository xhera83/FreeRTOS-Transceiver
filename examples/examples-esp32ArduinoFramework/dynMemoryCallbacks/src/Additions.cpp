#include "Additions.h"
#include "string.h"

void SENDER(void *)
{
    while(TASK_RECEIVER == nullptr || TASK_SENDER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_SENDER,1);
    #else
    FRTTCommunicationPartner partner[1];
    FRTTransceiver comm(TASK_SENDER,&partner[0],1);
    #endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_RECEIVER,nullptr,0,nullptr,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE);

    int * dummy = new int;
    std::string  * str1 = new std::string;
    std::string  * str2 = new std::string;

    *dummy = 15;
    str1->assign("ABC1");
    str2->assign("ABC2");

    comm.writeToQueue(TASK_RECEIVER,eINT,dummy,300,300,0);
    comm.writeToQueue(TASK_RECEIVER,eCPPSTR,str1,300,300,0);
    comm.writeToQueue(TASK_RECEIVER,eCPPSTR,str2,300,300,0);

    vTaskDelay(pdMS_TO_TICKS(1));
    
    delete dummy;
    delete str1;
    delete str2;

    vTaskDelay(5000);
    vTaskDelete(NULL);
}


void RECEIVER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_RECEIVER,1);
    #else
    FRTTCommunicationPartner partner[1];
    FRTTransceiver comm(TASK_RECEIVER,&partner[0],1);
    #endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);
    
    comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE,nullptr,0,nullptr,"QUEUE-TO-SENDER");

    /* We receive 3x data for which we make a real copy! The other task will destroy the data. */

    int i = 0;
    
    while(i < 3)
    {
        comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);
        i++;
    }

    vTaskDelay(pdMS_TO_TICKS(100)); /* SENDER_TASK will have deleted his data by now, but we made copies in time!! */

    int counter = comm.bufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
    printf("Amount of data received:  %d\n",counter);

    if(counter == 3)
    {   
        
        while(counter != 0)
        {
            const FRTTTempDataContainer * t = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,counter - 1);

            if(t != nullptr)
            {
                switch(t->u8DataType)
                {
                    case eINT:
                        printf("[int] Copied data by %p : %d\n",t->senderAddress,*static_cast<int *>(t->data));
                        break;
                    case eCPPSTR:
                        printf("[string] Copied data by %p: %s\n",t->senderAddress,static_cast<std::string * >(t->data)->c_str());
                        break;
                }
            }
            counter--;
        }
    }

    comm.delAllDatabuffForLine(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
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
     *                    ---> Use malloc/new (not recommended)
     *                    ---> Later implementations might provide some sort of internal memory pool implementation
     */ 

    internalBuffer.senderAddress = origingalContainer_onQueue.senderAddress;
    internalBuffer.u32AdditionalData = origingalContainer_onQueue.u32AdditionalData;
    internalBuffer.u8DataType = origingalContainer_onQueue.u8DataType;
    
    /* Here check if nullptr in originalContainer (even though its not possible), valid datatype range etc...*/

    switch (origingalContainer_onQueue.u8DataType)
    {
        case eINT:
            internalBuffer.data = new int;
            *(static_cast<int *>(internalBuffer.data)) = *(static_cast<int *>(origingalContainer_onQueue.data));

            break;
        case eCPPSTR:
            internalBuffer.data = new std::string;
            static_cast<std::string *>(internalBuffer.data)->assign(static_cast<std::string *>(origingalContainer_onQueue.data)->c_str());
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
     *               - Reverse the actions made in the allocator callback function (when malloc()/new used ---> free()/delete) 
     */
  
    switch(internalBuffer.u8DataType)
    {
        case eINT:
        case eCPPSTR:
            delete internalBuffer.data; 
            break;
    }
}