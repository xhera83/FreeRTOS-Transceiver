/*!
 * \file        SimpleEcho.ino
 * \brief       Echo communication
 * 
 * \details
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



FRTTTaskHandle TASK_ECHO;
FRTTQueueHandle ECHO_QUEUE;
FRTTSemaphoreHandle SEMAPHORE1; 

void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();

    ECHO_QUEUE = FRTTCreateQueue(QUEUELENGTH);

    SEMAPHORE1 = FRTTCreateSemaphore();

    xTaskCreatePinnedToCore(ECHO,"receiver-task",5000,nullptr,4,&TASK_ECHO,1);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}