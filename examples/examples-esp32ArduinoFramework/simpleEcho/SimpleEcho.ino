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