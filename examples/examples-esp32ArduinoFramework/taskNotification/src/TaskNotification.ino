/*!
 * \file        TaskNotification.ino
 * \brief       TaskNotification + data exchange between receiver and sender
 * 
 * \details     
 *              "WIRING":
 * 
 * 
 *                      ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄                                     ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 *                      █                █         → DATA                      █                      █
 *                      █    SENDER      █═════════════════════════════════════█       RECEIVER       █
 *                      █                █═════════════════════════════════════█                      █
 *                      █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█         ← Notification              █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
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

FRTTSemaphoreHandle SEMAPHORE1;


void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    QUEUE_TO_RECEIVER = FRTTCreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTCreateSemaphore();
    
    FRTTCreateTask(SENDER,"sender-task",5000,nullptr,5,&TASK_SENDER,0);
    FRTTCreateTask(RECEIVER,"receiver-task",5000,nullptr,4,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}