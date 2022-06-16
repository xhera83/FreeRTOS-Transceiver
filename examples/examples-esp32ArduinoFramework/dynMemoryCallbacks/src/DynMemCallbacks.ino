/*!
 * \file        DynMemCallbacks.ino
 * \brief       Dynamic memory allocation inside the callbacks to make a copy
 * 
 * \details     
 * 
 *              "WIRING":
 * 
 * 
 *                                  
 *                  ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄                  										▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 *                  █                █                                                          █                  █
 *                  █    SENDER      █                                                          █    RECEIVER      █
 *                  █                █══════════════════════════════════════════════════════════█                  █
 *                  █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█    				→ data = {std::string,int}				█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█ 
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

FRTTSemaphoreHandle SEMAPHORE;


void setup() {
    vTaskDelay(pdMS_TO_TICKS(200));
    
    printf("Setup() running.\n\n");

    QUEUE_TO_RECEIVER = FRTTCreateQueue(QUEUELENGTH);

    SEMAPHORE = FRTTCreateSemaphore();

    xTaskCreatePinnedToCore(SENDER,"sender-task",5000,nullptr,8,&TASK_SENDER,0);
    xTaskCreatePinnedToCore(RECEIVER,"receiver-task",5000,nullptr,8,&TASK_RECEIVER,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}