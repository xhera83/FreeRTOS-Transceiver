#include "Additions.h"

void SENDER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    const FRTTTaskHandle t = nullptr;

    /* 
        Notification val BIT0 ---> SEND uint8_t buffer
        Notification val BIT1 ---> SEND char * string
        Notification val BIT2 ---> SEND int buffer

    */


	#if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(TASK_SENDER,1);
	#else
	FRTTCommunicationPartner partner[1];
	FRTTransceiver comm(TASK_SENDER,&partner[0],1);
	#endif

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    int buffSizeInt = 5;
    int intBuff[buffSizeInt] = {15000,16000,17000,18000,19000};

    int buffSizeUInt = 2;
    uint8_t u8Buff[buffSizeUInt] = {127,144};

    char * cString = "This is a c-string";



    bool retVal = comm.addCommPartner(TASK_RECEIVER,nullptr,0,nullptr,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,"RECEIVER");
    
    uint32_t u32Notification = 0;
    for(;;)
    {
        if((u32Notification = comm.NotifyReceiveBasic(eFRTTNotifyActions::e_CLEARCOUNTONEXIT,200).getNotificationVal()) != 0)
        {   
            void * data;
            eDataTypes datatype;
            uint32_t u32AddData;

            if(u32Notification & (1UL << 0))
            {
                /* send uint8_t buffer, BIT0 was set*/
                
                data = &u8Buff[0];
                datatype = eUINT8BUFF;
                u32AddData = buffSizeUInt;
            }
            else if(u32Notification & (1UL << 1))
            {
                /* send char c-string */
                

                data = &cString[0];
                datatype = eCHARSTR;
                u32AddData = 0;
            }
            else if(u32Notification & (1UL << 2))
            {
                /* send int buffer */
                comm.writeToQueue(TASK_RECEIVER,eINTBUFF,&intBuff[0],500,500,buffSizeInt);

                data = &intBuff[0];
                datatype = eINTBUFF;
                u32AddData = buffSizeInt;
            }
            else if(u32Notification & (1UL << 3)){
                break;
            }
            else
            {
                continue;
            }
            comm.writeToQueue(TASK_RECEIVER,datatype,data,500,500,u32AddData);
        }
        else
        {
            // do some other work
        }

        comm.clearNotificationVal();
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    printf("\n\n");
    comm.printCommunicationsSummary();
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

    uint8_t u8Shift = 0;

    for(uint8_t u8I = 0; u8I < 67;u8I++)
    {
        comm.NotifyExtended(TASK_SENDER,eFRTTNotifyActions::e_SetValueWithoutOverwrite,(unsigned long)(1 << u8Shift++));

        if (u8Shift == 3) u8Shift = 0;

        if(comm.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX))
        {
            /* Here check data etc*/
            comm.delAllDatabuffForLine(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    comm.NotifyExtended(TASK_SENDER,eFRTTNotifyActions::e_SetValueWithOverwrite,(unsigned long)(1 << 3)); /* Is stop sequence */
    comm.printCommunicationsSummary();
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
     *                    ---> Use malloc (not recommended)
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
     *               - Reverse the actions made in the allocator callback function (if malloc() was used then ---> free()) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = nullptr;
    internalBuffer.data = nullptr;
}