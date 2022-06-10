/*!
 * \file        SimpleEcho.ino
 * \brief       Task talks to himself
 * 
 * \details     This example covers following topics:
 *                  - Setting up the communication
 *                  - Reading/Writing to/from a queue
 *                  - Checking how many messages on queue or in the buffer
 *                  - Reading/deleting buffered data
 *                  - "Bidirectional" communication / Echo communication
 *                  - Flush buffer
 * 
 * 
 *              "WIRING":
 * 
 * 
 * 
 *                                          ←→ DATA
 *                             ╔════════════════════════════════════╗
 *                             ║                                    ║
 *                             ║                                    ║
 *                  ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄                           ║
 *                  █                   █                           ║
 *                  █    ECHO TASK      █═══════════════════════════╝
 *                  █                   █
 *                  █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
 * 
 * 
 * 
 *                    
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <Arduino.h>
#include "Additions.h"

/* ######################################################################## EXAMPLE START ######################################################################## */



void ECHO(void *)
{
    while(TASK_ECHO == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(50)); /* So that no overlapping occurs if log_i()'s happen */

    FRTTransceiver comm(TASK_ECHO,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool retVal = comm.addCommPartner(TASK_ECHO,ECHO_QUEUE,QUEUELENGTH,SEMAPHORE1,ECHO_QUEUE,QUEUELENGTH,SEMAPHORE1,"ECHO");

    if(retVal)
    {
        log_i("Connected to myself...Echo without delay incoming...(Communication summary afterwards)");
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
    comm.printCommunicationsSummary();
    comm.~FRTTransceiver();
    vTaskDelete(NULL);
}



void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    ECHO_QUEUE = FRTTCreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTCreateSemaphore();

    xTaskCreatePinnedToCore(ECHO,"receiver-task",5000,NULL,4,&TASK_ECHO,1);
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
    internalBuffer.senderAddress = NULL;
    internalBuffer.data = NULL;
}