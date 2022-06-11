/*!
 * \file        BidirectionalComm.ino
 * \brief       Bidirectional communication between receiver and sender
 * 
 * \details     This example covers following topics:
 *                  - Setting up the communication
 *                  - Reading/Writing to/from a queue
 *                  - Checking how many messages on queue or in the buffer
 *                  - Reading/deleting buffered data
 *                  - Exchange of data between two tasks (one type of datatype)
 *                  - Both Queues have length 1
 *                  - TASK_SENDER will print its communication summary to console if set in FRTTransceiverSettings.h
 *                  
 *              Short code explanation:
 *                  - TASK_RECEIVER sends 9 times the COMMAND "COMMAND_SEND". 
 *                    TASK_RECEIVER sends the COMMAND "COMMAND_STOP" after the 10th time 
 *                    After sending COMMAND_SEND, TASK_RECEIVER will read a command received by TASK_SENDER
 * 
 *                  - TASK_SENDER waits for COMMAND "COMMAND_SEND" and sends back a command defined in the uin8_t array
 *                    TASK_SENDER stops after receiving the COMMAND "COMMAND_STOP"
 * 
 * 
 *              "WIRING":
 * 
 * 
 *                                  
 *                  ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄    ← command = {COMMAND_SEND,COMMAND_STOP}               ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 *                  █                █══════════════════════════════════════════════════════════█                  █
 *                  █    SENDER      █                                                          █    RECEIVER      █
 *                  █                █══════════════════════════════════════════════════════════█                  █
 *                  █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█    → command = {COMMAND_TURNLEDOFF,COMMAND_TURNLEDON}    █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█ 
 * 
 * 
 * 
 * 
 *                    
 *                      
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <Arduino.h>
#include "Additions.h"


/* ######################################################################## EXAMPLE START ######################################################################## */

void SENDER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));


    FRTTCommunicationPartner commStruct[1];

    FRTTransceiver comm(TASK_SENDER,&commStruct[0],1);

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_RECEIVER,QUEUE_FROM_RECEIVER,QUEUELENGTH,SEMAPHORE2,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,"RECEIVER");

    if(retVal)
    {
        log_i("Connected to communication line with the RECEIVER_TASK");
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
                        log_i("The receiver-task (%p) asked for instructions...",t->senderAddress);
                        comm.writeToQueue(TASK_RECEIVER,eCOMMAND,&u8COMMANDS[u8PosInBuff],100,100,u32AdditionalData);
                        u8PosInBuff++;
                    }
                    else if(u8COMMAND == COMMAND_STOP)
                    {
                        log_i("The receiver-task (%p) asked to stop the communication...",t->senderAddress);
                        comm.~FRTTransceiver();
                        break;
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(750));
    }
    //comm.printCommunicationsSummary();
    vTaskDelete(nullptr);
}

void RECEIVER(void *)
{
    while(TASK_SENDER == nullptr || TASK_RECEIVER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

    FRTTCommunicationPartner commStruct[1];
    FRTTransceiver comm(TASK_RECEIVER,&commStruct[0],1);

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_SENDER,QUEUE_TO_RECEIVER,QUEUELENGTH,SEMAPHORE1,QUEUE_FROM_RECEIVER,QUEUELENGTH,SEMAPHORE2,"SENDER");

    if(retVal)
    {
        log_i("Connected to communication line with the SENDER_TASK");
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
                    log_i("Data received by %p",t->senderAddress);
                    u8COMMAND = *((uint8_t *)t->data);

                    switch(u8COMMAND) 
                    {
                        case COMMAND_TURNLEDON:
                            log_i("Command received: turn led on");
                            break;
                        case COMMAND_TURNLEDOFF:
                            log_i("Command received: turn led off");
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
    comm.printCommunicationsSummary();
    vTaskDelete(nullptr);
}



void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    QUEUE_TO_RECEIVER = FRTTCreateQueue(QUEUELENGTH);
    QUEUE_FROM_RECEIVER = FRTTCreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTCreateSemaphore();
    SEMAPHORE2 = FRTTCreateSemaphore();

    xTaskCreatePinnedToCore(SENDER,"sender-task",25000,nullptr,5,&TASK_SENDER,0);
    xTaskCreatePinnedToCore(RECEIVER,"receiver-task",25000,nullptr,4,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
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