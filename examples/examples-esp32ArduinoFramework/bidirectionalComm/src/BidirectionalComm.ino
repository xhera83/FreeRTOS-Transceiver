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
 *                  - TASK_RECEIVER sends 9 times COMMAND "COMMAND_SEND". 
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

FRTTTaskHandle TASK_SENDER;
FRTTTaskHandle TASK_RECEIVER;

FRTTQueueHandle QUEUE_TO_RECEIVER;
FRTTQueueHandle QUEUE_FROM_RECEIVER;

FRTTSemaphoreHandle SEMAPHORE1;
FRTTSemaphoreHandle SEMAPHORE2;

void setup() {
    printf("Setup() running.\n\n");
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