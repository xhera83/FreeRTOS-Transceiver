/*!
 * \file        OneQueueMultipleUser.ino
 * \brief       Three tasks using the same queue (multi-sender-queue)
 * 
 * \details     
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