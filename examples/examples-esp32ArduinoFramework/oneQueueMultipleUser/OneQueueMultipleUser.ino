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


void UART_SLAVE(void *);
void SENSOR_SLAVE(void *);
void MOTOR_SLAVE(void *);

/* ######################################################################## EXAMPLE START ######################################################################## */

void Master(void *)
{
    while(TASK_MASTER == nullptr || TASK_MOTOR_SLAVE == nullptr ||
          TASK_SENSOR_SLAVE  == nullptr || TASK_UART_SLAVE == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    vTaskDelay(pdMS_TO_TICKS(1000));

    FRTTransceiver comm(TASK_MASTER,4);

    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    bool res = comm.addMultiSenderPartner(MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"Master-Slaves Channel");
    res = res && comm.addCommPartner(TASK_UART_SLAVE,nullptr,0,nullptr,QUEUE_TO_UART,QUEUELENGTH_GENERAL,SEMAPHORE_UART,"UART TASK");
    res = res && comm.addCommPartner(TASK_SENSOR_SLAVE,nullptr,0,nullptr,QUEUE_TO_SENSOR,QUEUELENGTH_GENERAL,SEMAPHORE_SENSOR,"SENSOR TASK");
    res = res && comm.addCommPartner(TASK_MOTOR_SLAVE,nullptr,0,nullptr,QUEUE_TO_MOTOR,QUEUELENGTH_GENERAL,SEMAPHORE_MOTOR,"UART TASK");

    if(res)
    {
        log_i("Connected to multi-sender-queueue and 3 others");
    }
    else
    {
        log_i("Something went wrong!");
        comm.~FRTTransceiver();
        vTaskDelete(nullptr);
    }

    uint8_t u8CommandPos = 0;
    uint8_t u8Commands[COMMANDS] = {COMMAND_SEND,COMMAND_SLEEP,COMMAND_REFRESHDATA,COMMAND_SEND,COMMAND_SEND,COMMAND_SLEEP,
                                    COMMAND_SLEEP,COMMAND_SLEEP,COMMAND_SEND,COMMAND_SEND,COMMAND_REFRESHDATA,COMMAND_REFRESHDATA,
                                    COMMAND_SEND,COMMAND_SEND,COMMAND_STOP};

    comm.databroadcast(eCOMMAND,&u8Commands[u8CommandPos++],FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,0);

    uint8_t u8PackagesReceived = 0;

    for(;;)
    {   
        if(comm.messagesOnQueue(eMultiSenderQueue::eMULTISENDERQ0) > 0)
        {   

            res = comm.readFromQueue(nullptr,eMultiSenderQueue::eMULTISENDERQ0,false,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);

            if(res)
            {   
                u8PackagesReceived++;
                const FRTTTempDataContainer * t = comm.getOldestBufferedDataFrom(nullptr,eMultiSenderQueue::eMULTISENDERQ0,false);

                if(t != nullptr)
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
                comm.delAllDatabuffForLine(nullptr,eMultiSenderQueue::eMULTISENDERQ0,false);
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
    vTaskDelete(nullptr);
}

void UART_SLAVE(void *)
{
    while(TASK_MASTER == nullptr || TASK_UART_SLAVE == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    FRTTransceiver comm(TASK_UART_SLAVE,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_MASTER,QUEUE_TO_UART,QUEUELENGTH_GENERAL,SEMAPHORE_UART,MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"MASTER");

    uint8_t u8BuffLength = 10;
    int buffer[u8BuffLength] = {0x0124,0x1222,0xFF,0x324,0x42,0x523,0x642,0x752,0x823,0x912};

    handleSlaveWork(&comm,&buffer[0],u8BuffLength,TASK_MASTER,eUARTBUFFER);

    vTaskDelay(pdMS_TO_TICKS(3000));
    vTaskDelete(nullptr);
}

void SENSOR_SLAVE(void *)
{
    while(TASK_MASTER == nullptr || TASK_SENSOR_SLAVE == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    FRTTransceiver comm(TASK_UART_SLAVE,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_MASTER,QUEUE_TO_SENSOR,QUEUELENGTH_GENERAL,SEMAPHORE_SENSOR,MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"MASTER");

    uint8_t u8BuffLength = 5;
    int buffer[u8BuffLength] = {179,183,240,120,111,198};

    handleSlaveWork(&comm,&buffer[0],u8BuffLength,TASK_MASTER,eSENSORBUFFER);

    vTaskDelay(pdMS_TO_TICKS(3000));
    vTaskDelete(nullptr);
}

void MOTOR_SLAVE(void *)
{
    while(TASK_MASTER == nullptr || TASK_MOTOR_SLAVE == nullptr) vTaskDelay(pdMS_TO_TICKS(1));
    FRTTransceiver comm(TASK_UART_SLAVE,1);
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(dataDestroyer);

    comm.addCommPartner(TASK_MASTER,QUEUE_TO_MOTOR,QUEUELENGTH_GENERAL,SEMAPHORE_MOTOR,MULTISENDERQ,QUEUELENGTH_MULTISENDERQ,SEMAPHORE_MULTIQ,"MASTER");

    uint8_t u8BuffLength = 6;
    int buffer[u8BuffLength] = {1700,1900,2100,1900,1400,1300};

    handleSlaveWork(&comm,&buffer[0],u8BuffLength,TASK_MASTER,eMOTORBUFFER);

    vTaskDelay(pdMS_TO_TICKS(3000));
    vTaskDelete(nullptr);
}

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



void dataAllocator (const FRTTDataContainerOnQueue & origingalContainer_onQueue ,FRTTTempDataContainer & internalBuffer){

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and data destroyer callback function.
     *      
     *      To do:
     *          
     *          (1): 
     *               - Copy u8Datatype variable
     *               - Copy additionalData variable
     *               - Copy senderAdress variable
     *          
     *          (2): 
     *               - Provide some sort of way to copy the main data over:
     *                    ---> Just copy the pointer over
     *                    ---> Use malloc (not recommended)
     *                    ---> Later implementations might provide some sort of internal memory pool implementation
     */ 

    internalBuffer.u8DataType = origingalContainer_onQueue.u8DataType;
    internalBuffer.u32AdditionalData = origingalContainer_onQueue.u32AdditionalData;
    internalBuffer.senderAddress = origingalContainer_onQueue.senderAddress;
    internalBuffer.data = origingalContainer_onQueue.data;
}

void dataDestroyer(FRTTTempDataContainer & internalBuffer) {

    /**
     *      In order to use the library in its current version you need to supply both a
     *      data allocator and data destroyer callback function.
     *      
     *      To do:
     *          
     *          (1): 
     *               - Reverse the actions made in the allocator callback function (malloc() ---> free()) 
     */

    internalBuffer.u8DataType = 0;
    internalBuffer.u32AdditionalData = 0;
    internalBuffer.senderAddress = nullptr;
    internalBuffer.data = nullptr;
}


String printBuffer(int * u8Buffer, uint8_t u8Length)
{   
    String temp("");

    temp += "BUFFER["; temp += u8Length; temp += "] : {";

    for(uint8_t u8J = 0;u8J < u8Length;u8J++)
    {
        temp += u8Buffer[u8J];
        if(u8J == u8Length - 1)
        {
            temp += "}";
        }
        else
        {
            temp += ",";
        }
    }
    return temp;
}

void handleSlaveWork(FRTTransceiver * comm,int * buffer,uint8_t u8Length,FRTTTaskHandle partnertask,eDataTypes datatype)
{
    for(;;)
    {
        
        /* Check for command */
        bool res = comm->readFromQueue(partnertask,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);
        if(res)
        {   
            const FRTTTempDataContainer * t = comm->getOldestBufferedDataFrom(partnertask,eMultiSenderQueue::eNOMULTIQSELECTED,true);

            if(t != nullptr)
            {
                if(t->u8DataType == eCOMMAND)
                {
                    //log_i("Received an instruction by task %p",t->senderAddress);
                    switch(*((uint8_t *)t->data))
                    {
                        case COMMAND_SEND:
                            comm->writeToQueue(partnertask,datatype,&buffer[0],FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX,u8Length);
                            break;
                        case COMMAND_REFRESHDATA:
                            /* Changing data */
                            for(uint8_t u8I = 0; u8I < u8Length;u8I++)buffer[u8I] = buffer[u8I] + 5;
                            break;
                        case COMMAND_STOP:
                            comm->delAllDatabuffForLine(partnertask,eMultiSenderQueue::eNOMULTIQSELECTED,true);
                            comm->~FRTTransceiver();
                            break;
                        case COMMAND_SLEEP:
                            vTaskDelay(SLEEP_MS);
                            break;
                        default:
                            break;
                    }
                    comm->delAllDatabuffForLine(partnertask,eMultiSenderQueue::eNOMULTIQSELECTED,true);
                }
            }
        }
    }
}