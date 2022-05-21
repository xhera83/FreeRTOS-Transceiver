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
#include <include/Additions.h>


/* ######################################################################## EXAMPLE START ######################################################################## */

void SENDER(void *)
{
    while(TASK_SENDER == NULL || TASK_RECEIVER == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    FRTTransceiver comm(TASK_SENDER,1);
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
            bool retVal = comm.readFromQueue(TASK_RECEIVER,eNOMULTIQSELECTED,true,100,100);

            if(retVal && comm.hasDataFrom(TASK_RECEIVER,eNOMULTIQSELECTED,true))
            {
                const FRTTransceiver_TempDataContainer * t = comm.getNewestBufferedDataFrom(TASK_RECEIVER,eNOMULTIQSELECTED,true);
                
                if(t != NULL)
                {
                    uint8_t u8COMMAND = *((uint8_t *)t->data);

                    comm.manualDeleteAllocatedDatabufferForLine(TASK_RECEIVER,eNOMULTIQSELECTED,true,0);

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
    vTaskDelete(NULL);
}

void RECEIVER(void *)
{
    while(TASK_SENDER == NULL || TASK_RECEIVER == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

    FRTTransceiver comm(TASK_RECEIVER,1);
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
            bool retVal = comm.readFromQueue(TASK_SENDER,eNOMULTIQSELECTED,true,100,100);

            if(retVal && comm.hasDataFrom(TASK_SENDER,eNOMULTIQSELECTED,true))
            {
                const FRTTransceiver_TempDataContainer * t = comm.getNewestBufferedDataFrom(TASK_SENDER,eNOMULTIQSELECTED,true);
                
                if(t != NULL)
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
                    comm.manualDeleteAllAllocatedDatabuffersForLine(TASK_SENDER,eNOMULTIQSELECTED,true);
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
    comm.~FRTTransceiver();
    vTaskDelete(NULL);
}



void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    QUEUE_TO_RECEIVER = FRTTransceiver_CreateQueue(QUEUELENGTH);
    QUEUE_FROM_RECEIVER = FRTTransceiver_CreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTransceiver_CreateSemaphore();
    SEMAPHORE2 = FRTTransceiver_CreateSemaphore();

    xTaskCreatePinnedToCore(SENDER,"sender-task",25000,NULL,5,&TASK_SENDER,0);
    xTaskCreatePinnedToCore(RECEIVER,"receiver-task",25000,NULL,4,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}

