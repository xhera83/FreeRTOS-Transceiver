/*!
 * \file        Additions.h
 * \brief       Additional data for the examples
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include <FRTTransceiver.h>

/* datatypes recognized throughout the example */
typedef enum
{
   eUARTBUFFER = 0,
   eSENSORBUFFER = 2,
   eMOTORBUFFER = 3,
   eCOMMAND = 4,
}eDataTypes;

FRTTransceiver_TaskHandle TASK_MASTER;
FRTTransceiver_TaskHandle TASK_UART_SLAVE;
FRTTransceiver_TaskHandle TASK_MOTOR_SLAVE;
FRTTransceiver_TaskHandle TASK_SENSOR_SLAVE;

FRTTransceiver_QueueHandle MULTISENDERQ;
FRTTransceiver_QueueHandle QUEUE_TO_MOTOR;
FRTTransceiver_QueueHandle QUEUE_TO_SENSOR;
FRTTransceiver_QueueHandle QUEUE_TO_UART;

FRTTransceiver_SemaphoreHandle SEMAPHORE_MULTIQ;
FRTTransceiver_SemaphoreHandle SEMAPHORE_MOTOR;
FRTTransceiver_SemaphoreHandle SEMAPHORE_UART;
FRTTransceiver_SemaphoreHandle SEMAPHORE_SENSOR;

#define QUEUELENGTH_MULTISENDERQ        (2u)    // multi-sender queuelength
#define QUEUELENGTH_GENERAL             (1u)    // queuelength of all other queues

#define COMMAND_SEND                    (0u)    // send current stored values
#define COMMAND_REFRESHDATA             (1u)    // read new values from uart|motor|sensor
#define COMMAND_STOP                    (2u)    // stop your task
#define COMMAND_SLEEP                   (3u)    // send tasks to sleep

#define SLEEP_MS                        (1200u)
#define COMMANDS                        (15u)

void dataAllocator (const FRTTransceiver_DataContainerOnQueue & origingalContainer_onQueue ,FRTTransceiver_TempDataContainer & internalBuffer){

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

void dataDestroyer(FRTTransceiver_TempDataContainer & internalBuffer) {

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
    internalBuffer.senderAddress = NULL;
    internalBuffer.data = NULL;
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

void handleSlaveWork(FRTTransceiver * comm,int * buffer,uint8_t u8Length,FRTTransceiver_TaskHandle partnertask,eDataTypes datatype)
{
    for(;;)
    {
        
        /* Check for command */
        bool res = comm->readFromQueue(partnertask,eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,FRTTRANSCEIVER_WAITMAX);
        if(res)
        {   
            const FRTTransceiver_TempDataContainer * t = comm->getOldestBufferedDataFrom(partnertask,eNOMULTIQSELECTED,true);

            if(t != NULL)
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
                            comm->manualDeleteAllAllocatedDatabuffersForLine(partnertask,eNOMULTIQSELECTED,true);
                            comm->~FRTTransceiver();
                            break;
                        case COMMAND_SLEEP:
                            vTaskDelay(SLEEP_MS);
                            break;
                        default:
                            break;
                    }
                    comm->manualDeleteAllAllocatedDatabuffersForLine(partnertask,eNOMULTIQSELECTED,true);
                }
            }
        }
    }
}