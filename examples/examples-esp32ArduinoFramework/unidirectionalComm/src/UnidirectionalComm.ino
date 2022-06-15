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
#include "Additions.h"

/* ######################################################################## EXAMPLE START ######################################################################## */

FRTTTaskHandle TASK_SENDER;
FRTTTaskHandle TASK_RECEIVER;

FRTTQueueHandle QUEUE_TO_RECEIVER;

FRTTSemaphoreHandle SEMAPHORE1;

void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    QUEUE_TO_RECEIVER = FRTTCreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTCreateSemaphore();

    xTaskCreatePinnedToCore(SENDER,"sender-task",5000,nullptr,5,&TASK_SENDER,0);
    xTaskCreatePinnedToCore(RECEIVER,"receiver-task",5000,nullptr,4,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}