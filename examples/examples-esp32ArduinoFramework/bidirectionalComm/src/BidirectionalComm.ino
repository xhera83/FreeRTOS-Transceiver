/*!
 * \file        BidirectionalComm.ino
 * \brief       Bidirectional communication between receiver and sender
 * 
 * \details     
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