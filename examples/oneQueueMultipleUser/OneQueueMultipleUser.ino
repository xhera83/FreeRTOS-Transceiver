/*!
 * \file        multiDatatypeOnQueue.ino
 * \brief       Multiple datatypes simultanously on the same queue
 * 
 * \details     This example covers following topics:
 *                  - Setting up the communication
 *                  - Reading/Writing to/from a queue
 *                  - Checking how many messages on queue or in the buffer
 *                  - Reading/deleting buffered data
 *                  - Unidirectional communication
 *                  - Muliple datatypes the same time on one queue
 *                  - Check if datatype in buffer
 *                  - Flush buffer
 *                    
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */

#include <Arduino.h>
#include <include/Additions.h>


void UART_SLAVE(void *);
void SENSOR_SLAVE(void *);
void MOTOR_SLAVE(void *);

/* ######################################################################## EXAMPLE START ######################################################################## */

void Master(void *)
{
    while(TASK_MASTER == NULL || TASK_MOTOR_SLAVE == NULL ||
          TASK_SENSOR_SLAVE  == NULL || TASK_UART_SLAVE == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(1000));

    FRTTransceiver comm(TASK_MASTER,4);

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool res = comm.addMultiSenderReadOnlyQueue(MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"Master-Slaves Channel");
    res = res && comm.addCommPartner(TASK_UART_SLAVE,NULL,0,NULL,QUEUE_TO_UART,QUEUELENGTH_GENERAL,SEMAPHORE_UART,"UART TASK");
    res = res && comm.addCommPartner(TASK_SENSOR_SLAVE,NULL,0,NULL,QUEUE_TO_SENSOR,QUEUELENGTH_GENERAL,SEMAPHORE_SENSOR,"SENSOR TASK");
    res = res && comm.addCommPartner(TASK_MOTOR_SLAVE,NULL,0,NULL,QUEUE_TO_MOTOR,QUEUELENGTH_GENERAL,SEMAPHORE_MOTOR,"UART TASK");

    if(res)
    {
        log_i("Connected to multi-sender-queueue and 3 others");
    }
    else
    {
        log_i("Something went wrong!");
        comm.~FRTTransceiver();
        vTaskDelete(NULL);
    }

    uint8_t u8CommandPos = 0;
    uint8_t u8Commands[COMMANDS] = {COMMAND_SEND,COMMAND_SLEEP,COMMAND_REFRESHDATA,COMMAND_SEND,COMMAND_SEND,COMMAND_SLEEP,
                                    COMMAND_SLEEP,COMMAND_SLEEP,COMMAND_SEND,COMMAND_SEND,COMMAND_REFRESHDATA,COMMAND_REFRESHDATA,
                                    COMMAND_SEND,COMMAND_SEND,COMMAND_STOP};

    comm.databroadcast(eCOMMAND,&u8Commands[u8CommandPos++],FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,0);

    uint8_t u8PackagesReceived = 0;

    for(;;)
    {   
        if(comm.messagesOnQueue(eMULTISENDERQ0) > 0)
        {   

            res = comm.readFromQueue(NULL,eMULTISENDERQ0,false,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);

            if(res)
            {   
                u8PackagesReceived++;
                const TempDataContainer * t = comm.getOldestBufferedDataFrom(NULL,eMULTISENDERQ0,false);

                if(t != NULL)
                {   
                    uint8_t u8LengthOfBuffers = t->u32AdditionalData;
                    String datatype("");

                    switch(t->u8DataType)
                    {
                        case eUARTBUFFER:
                            datatype = "UART DATA";
                            break;
                        case eSENSORBUFFER:
                            datatype = "SENSOR DATA";
                            break;
                        case eMOTORBUFFER:
                            datatype = "MOTOR DATA";
                            break;  
                        default:
                            break;  
                    }

                    log_i("Master task received %s by task %p",datatype.c_str(),t->senderAddress);
                    String temp = printBuffer((int * )t->data,t->u32AdditionalData);
                    log_i("%s",temp.c_str());
                }
                comm.manualDeleteAllAllocatedDatabuffersForLine(NULL,eMULTISENDERQ0,false);
            }
        }
        else if(u8PackagesReceived == 3 || u8Commands[u8CommandPos-1] == COMMAND_REFRESHDATA || u8Commands[u8CommandPos-1] == COMMAND_SLEEP)
        {
            u8PackagesReceived = 0;

            bool res = comm.databroadcast(eCOMMAND,&u8Commands[u8CommandPos],FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,0);

            log_i("");

            if(u8Commands[u8CommandPos] == COMMAND_STOP)
            {
                break;
            }
            u8CommandPos++;
        }
    }

    vTaskDelay(pdMS_TO_TICKS(3000));
    comm.printCommunicationsSummary();
    comm.~FRTTransceiver();
    vTaskDelete(NULL);
}

void UART_SLAVE(void *)
{
    while(TASK_MASTER == NULL || TASK_UART_SLAVE == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    FRTTransceiver comm(TASK_UART_SLAVE,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_MASTER,QUEUE_TO_UART,QUEUELENGTH_GENERAL,SEMAPHORE_UART,MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"MASTER");

    uint8_t u8BuffLength = 10;
    int buffer[u8BuffLength] = {0x0124,0x1222,0xFF,0x324,0x42,0x523,0x642,0x752,0x823,0x912};

    handleSlaveWork(&comm,&buffer[0],u8BuffLength,TASK_MASTER,eUARTBUFFER);

    vTaskDelay(pdMS_TO_TICKS(3000));
    vTaskDelete(NULL);
}

void SENSOR_SLAVE(void *)
{
    while(TASK_MASTER == NULL || TASK_SENSOR_SLAVE == NULL) vTaskDelay(pdMS_TO_TICKS(1));

    FRTTransceiver comm(TASK_UART_SLAVE,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_MASTER,QUEUE_TO_SENSOR,QUEUELENGTH_GENERAL,SEMAPHORE_SENSOR,MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"MASTER");

    uint8_t u8BuffLength = 5;
    int buffer[u8BuffLength] = {179,183,240,120,111,198};

    handleSlaveWork(&comm,&buffer[0],u8BuffLength,TASK_MASTER,eSENSORBUFFER);

    vTaskDelay(pdMS_TO_TICKS(3000));
    vTaskDelete(NULL);
}

void MOTOR_SLAVE(void *)
{
    while(TASK_MASTER == NULL || TASK_MOTOR_SLAVE == NULL) vTaskDelay(pdMS_TO_TICKS(1));
    FRTTransceiver comm(TASK_UART_SLAVE,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_MASTER,QUEUE_TO_MOTOR,QUEUELENGTH_GENERAL,SEMAPHORE_MOTOR,MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"MASTER");

    uint8_t u8BuffLength = 6;
    int buffer[u8BuffLength] = {1700,1900,2100,1900,1400,1300};

    handleSlaveWork(&comm,&buffer[0],u8BuffLength,TASK_MASTER,eMOTORBUFFER);

    vTaskDelay(pdMS_TO_TICKS(3000));
    vTaskDelete(NULL);
}

void setup() {
    log_i("Setup() running.\n\n");
    disableCore0WDT();
    
    MULTISENDERQ = FRTTransceiver_CreateQueue(QUEUELENGTH_MULTISENDERQ,sizeof(struct DataContainerOnQueue));
    QUEUE_TO_MOTOR = FRTTransceiver_CreateQueue(QUEUELENGTH_GENERAL,sizeof(struct DataContainerOnQueue));
    QUEUE_TO_UART = FRTTransceiver_CreateQueue(QUEUELENGTH_GENERAL,sizeof(struct DataContainerOnQueue));
    QUEUE_TO_SENSOR = FRTTransceiver_CreateQueue(QUEUELENGTH_GENERAL,sizeof(struct DataContainerOnQueue));
    
    SEMAPHORE_MULTIQ = FRTTransceiver_CreateSemaphore();
    SEMAPHORE_MOTOR = FRTTransceiver_CreateSemaphore();
    SEMAPHORE_UART = FRTTransceiver_CreateSemaphore();
    SEMAPHORE_SENSOR = FRTTransceiver_CreateSemaphore();
    
    xTaskCreatePinnedToCore(Master,"master",5000,NULL,4,&TASK_MASTER,1);

    xTaskCreatePinnedToCore(SENSOR_SLAVE,"sensor-slave",5000,NULL,5,&TASK_SENSOR_SLAVE,0);
    xTaskCreatePinnedToCore(UART_SLAVE,"uart-slave",5000,NULL,5,&TASK_UART_SLAVE,0);
    xTaskCreatePinnedToCore(MOTOR_SLAVE,"motor-slave",5000,NULL,5,&TASK_MOTOR_SLAVE,0);
}

/* This loop is running when no other task is on */
void loop() {
    delay(10000);
}