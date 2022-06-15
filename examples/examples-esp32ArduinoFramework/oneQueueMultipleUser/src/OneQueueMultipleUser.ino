/*!
 * \file        OneQueueMultipleUser.ino
 * \brief       Three tasks using the same queue (multi-sender-queue)
 * 
 * \details     This example covers following topics:
 *                  - Setting up the communication
 *                  - Reading/Writing to/from a queue
 *                  - databroadcasting
 *                  - Checking how many messages on queue or in the buffer
 *                  - Reading/deleting buffered data
 *                  - Unidirectional communication & usage of a multi-sender-queue
 *                  - Muliple datatypes on one queue at the same time
 *                  - Flush buffer
 *              
 *              Short code explanation:
 *                  - TASK_MASTER has a pre defined set of COMMANDS (defined in Additions.h) which he broadcasts to the other tasks (slaves)
 *                    TASK_MASTER then receives databuffers with some random values
 *                    The last COMMAND is COMMAND_STOP which stops all the communication and prints a summary of the communication(s) afterwards
 * 
 * 
 * 
 *              "WIRING":
 *                                            → COMMANDS
 *                                    ╔═══════════════════════════════════════════════════════════════════════╗          
 *                                    ║                                                                       ║
 *                                    ║                                              ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄    ║
 *                                    ║                                              █                   █    ║
 *                                    ║                                         ║════█    MOTOR SLAVE    █════╝
 *                                    ║                                         ║    █                   █
 *                                    ║                                         ║    █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█
 *                                    ║                                         ║    
 *                    ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄                                      ║    ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄                        
 *                    █                  █     ← Data (multi-sender-queue)      ║    █                   █          
 *                    █     MASTER       █══════════════════════════════════════╠════█    UART SLAVE     █═══════════╗
 *                    █                  █                                      ║    █                   █           ║
 *                    █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█                                      ║    █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█           ║
 *                          ║         ║                                         ║                                    ║
 *                          ║         ║                                         ║    ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄           ║   
 *                          ║         ║                                         ║    █                   █           ║
 *                          ║         ║                                         ║════█    SENSOR SLAVE   █═══════╗   ║
 *                          ║         ║                                              █                   █       ║   ║
 *                          ║         ║       → COMMANDS                             █▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄█       ║   ║
 *                          ║         ╚══════════════════════════════════════════════════════════════════════════╝   ║
 *                          ║                 → COMMANDS                                                             ║      
 *                          ╚════════════════════════════════════════════════════════════════════════════════════════╝
 * 
 * 
 * 
 * 
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <Arduino.h>
#include "Additions.h"


/* ######################################################################## EXAMPLE START ######################################################################## */

FRTTTaskHandle TASK_MASTER;
FRTTTaskHandle TASK_UART_SLAVE;
FRTTTaskHandle TASK_MOTOR_SLAVE;
FRTTTaskHandle TASK_SENSOR_SLAVE;

FRTTQueueHandle MULTISENDERQ;
FRTTQueueHandle QUEUE_TO_MOTOR;
FRTTQueueHandle QUEUE_TO_SENSOR;
FRTTQueueHandle QUEUE_TO_UART;

FRTTSemaphoreHandle SEMAPHORE_MULTIQ;
FRTTSemaphoreHandle SEMAPHORE_MOTOR;
FRTTSemaphoreHandle SEMAPHORE_UART;
FRTTSemaphoreHandle SEMAPHORE_SENSOR;

void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();
    
    MULTISENDERQ = FRTTCreateQueue(QUEUELENGTH_MULTISENDERQ);
    QUEUE_TO_MOTOR = FRTTCreateQueue(QUEUELENGTH_GENERAL);
    QUEUE_TO_UART = FRTTCreateQueue(QUEUELENGTH_GENERAL);
    QUEUE_TO_SENSOR = FRTTCreateQueue(QUEUELENGTH_GENERAL);
    
    SEMAPHORE_MULTIQ = FRTTCreateSemaphore();
    SEMAPHORE_MOTOR = FRTTCreateSemaphore();
    SEMAPHORE_UART = FRTTCreateSemaphore();
    SEMAPHORE_SENSOR = FRTTCreateSemaphore();
    
    xTaskCreatePinnedToCore(Master,"master",5000,nullptr,4,&TASK_MASTER,1);

    xTaskCreatePinnedToCore(SENSOR_SLAVE,"sensor-slave",5000,nullptr,5,&TASK_SENSOR_SLAVE,0);
    xTaskCreatePinnedToCore(UART_SLAVE,"uart-slave",5000,nullptr,5,&TASK_UART_SLAVE,0);
    xTaskCreatePinnedToCore(MOTOR_SLAVE,"motor-slave",5000,nullptr,5,&TASK_MOTOR_SLAVE,0);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}