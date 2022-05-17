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
#include <include/Additions.h>

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
            bool res = comm.readFromQueue(TASK_ECHO,eNOMULTIQSELECTED,true,0,0);
            comm.manualDeleteAllAllocatedDatabuffersForLine(TASK_ECHO,eNOMULTIQSELECTED,true);
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

    ECHO_QUEUE = FRTTransceiver_CreateQueue(QUEUELENGTH,sizeof(struct DataContainerOnQueue));

    SEMAPHORE1 = FRTTransceiver_CreateSemaphore();

    xTaskCreatePinnedToCore(ECHO,"receiver-task",5000,NULL,4,&TASK_ECHO,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}