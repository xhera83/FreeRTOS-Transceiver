/*!
 * \file        UnidirectionalComm.ino
 * \brief       Unidrectional communication between receiver and sender
 * 
 * \details     This example covers following topics:
 *                  - Setting up the communication
 *                  - Reading/Writing to/from a queue
 *                  - Checking how many messages on queue or in the buffer
 *                  - Reading/deleting buffered data
 *                  - Unidirectional communication
 *                  - RECEIVER_TASK also reads u32AdditionalData to check when to stop itself
 *                  - Both Queues have length 1
 * 
 *  
 *              "WIRING":
 * 
 * 
 *                      ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄                                     ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 *                      █                █         → DATA                      █                      █
 *                      █    SENDER      █═════════════════════════════════════█       RECEIVER       █
 *                      █                █                                     █                      █
 *                      █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█                                     █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
 * 
 * 
 *                    
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <Arduino.h>
#include <include/Additions.h>

void dataAllocator (const FRTTransceiver_DataContainerOnQueue & origingalContainer_onQueue ,FRTTransceiver_TempDataContainer & internalBuffer){

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
}

void dataDestroyer(FRTTransceiver_TempDataContainer & internalBuffer) {

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
    internalBuffer.data = NULL;
}

/* ######################################################################## EXAMPLE START ######################################################################## */

void SENDER(void *)
{
    while(TASK_SENDER == NULL || TASK_RECEIVER == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    FRTTransceiver comm(TASK_SENDER,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_RECEIVER,NULL,0,NULL,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,"RECEIVER");

    if(retVal)
    {
        log_i("Connected to communication line with the RECEIVER_TASK");
    }


    char *messages[] = {"First message","Second message","Third message","Fourth message","Fifth message"};
    uint32_t u32AdditionalData = 0;
    
    for(uint8_t u8I = 0;u8I < 5 ;u8I++)
    {
        bool res = comm.writeToQueue(TASK_RECEIVER,eMESSAGE,&messages[u8I][0],FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,++u32AdditionalData);

        if(res)
        {
            log_i("Data sent...");
        }
        else 
        {
            log_i("Couldnt send the data...");
        }
        vTaskDelay(pdMS_TO_TICKS(750));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    //comm.printCommunicationsSummary();
    vTaskDelete(NULL);
}

void RECEIVER(void *)
{
    while(TASK_SENDER == NULL || TASK_RECEIVER == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

    FRTTransceiver comm(TASK_RECEIVER,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,1,SEMAPHORE1,NULL,0,NULL,"SENDER");

    if(retVal)
    {
        log_i("Connected to communication line with the SENDER_TASK");
    }
    
    uint32_t u32AdditionalData = 0;
    
    for(;;)
    {
        if(comm.messagesOnQueue(TASK_SENDER,false) > 0)
        {
            bool res = comm.readFromQueue(TASK_SENDER,eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);

            if(res)
            {
                const FRTTransceiver_TempDataContainer * t = comm.getBufferedDataFrom(TASK_SENDER,eNOMULTIQSELECTED,true,0);
                
                if(t != NULL)
                {
                    log_i("Received data from : %p",t->senderAddress);
                    
                    if(t->u8DataType == eMESSAGE)
                    {
                        log_i("DATA [Message]: %s",(char *)t->data);
                        u32AdditionalData = t->u32AdditionalData;
                        log_i("Additional data: %d",u32AdditionalData);
                    }
                    comm.manualDeleteAllAllocatedDatabuffersForLine(TASK_SENDER,eNOMULTIQSELECTED,true);
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
    comm.printCommunicationsSummary();
    comm.~FRTTransceiver();
    vTaskDelete(NULL);
}



void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    QUEUE_TO_RECEIVER = FRTTransceiver_CreateQueue(QUEUELENGTH,sizeof(struct FRTTransceiver_DataContainerOnQueue));

    SEMAPHORE1 = FRTTransceiver_CreateSemaphore();

    xTaskCreatePinnedToCore(SENDER,"sender-task",5000,NULL,5,&TASK_SENDER,0);
    xTaskCreatePinnedToCore(RECEIVER,"receiver-task",5000,NULL,4,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}