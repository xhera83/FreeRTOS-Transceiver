/*!
 * \file        MultiDatatypeOnQueue.ino
 * \brief       Multiple datatypes simultanously on the same queue
 * 
 * \details     This example covers following topics:
 *                  - Setting up the communication
 *                  - Reading/Writing to/from a queue
 *                  - Checking how many messages on queue or in the buffer
 *                  - Reading/deleting buffered data
 *                  - Unidirectional communication
 *                  - Muliple datatypes the same time on one queue
 *                  - Check if datatype in buffer
 *                  - Flush buffer
 * 
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
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <Arduino.h>
#include "Additions.h"

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
            log_i("All data transmitted!");
        }

        vTaskDelay(pdMS_TO_TICKS(700));
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    //comm.printCommunicationsSummary();
    comm.~FRTTransceiver();
    vTaskDelete(NULL);
}

void RECEIVER(void *)
{
    while(TASK_SENDER == NULL || TASK_RECEIVER == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

    FRTTransceiver comm(TASK_RECEIVER,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,NULL,0,NULL,"SENDER");

    if(retVal)
    {
        log_i("Connected to communication line with the SENDER_TASK");
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
                log_i("All data read into internal buffers");
                const FRTTransceiver_TempDataContainer * t1 = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0);
                const FRTTransceiver_TempDataContainer * t2 = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,1);
                const FRTTransceiver_TempDataContainer * t3 = comm.getBufferedDataFrom(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,2);
                
                if(t1 && t2 && t3)
                {
                    log_i("Package sender addresses (Package[1]:%p, Package[2]:%p, Package[3]:%p)",t1->senderAddress,t2->senderAddress,t3->senderAddress);
                    log_i("Additional data for each received package: Package[1]: %d Package[2]: %d Package[3]: %d",t1->u32AdditionalData,t2->u32AdditionalData,
                                                                                                                                            t3->u32AdditionalData);
                    log_i("Occurence of eMESSAGE in buffer: %d",comm.checkIfDataTypeInBuffer(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,eMESSAGE));
                    log_i("Occurence of eSTRUCT in buffer: %d",comm.checkIfDataTypeInBuffer(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,eSTRUCT));
                    log_i("Occurence of eINT in buffer: %d",comm.checkIfDataTypeInBuffer(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,eINT));
                    u8I++;
                }

                comm.manualDeleteAllAllocatedDatabuffersForLine(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true);
            }
        }
        if(u8I == 7)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(60));
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    comm.printCommunicationsSummary();
    comm.~FRTTransceiver();
    vTaskDelete(NULL);
}



void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    QUEUE_TO_RECEIVER = FRTTransceiver_CreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTransceiver_CreateSemaphore();

    xTaskCreatePinnedToCore(SENDER,"sender-task",5000,NULL,5,&TASK_SENDER,0);
    xTaskCreatePinnedToCore(RECEIVER,"receiver-task",5000,NULL,4,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}