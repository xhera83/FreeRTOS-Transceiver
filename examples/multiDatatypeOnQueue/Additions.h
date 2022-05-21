/*!
 * \file        Additions.h
 * \brief       Additional data for the examples
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 */


#include <FRTTransceiver.h>

/* datatypes recognized throughout the example */
typedef enum
{
   eMESSAGE = 0,
   eINT = 1,
   eSTRUCT = 2,

}eDataTypes;

struct TestDType
{
   int var1;
   int var2;
};

FRTTransceiver_TaskHandle TASK_SENDER;
FRTTransceiver_TaskHandle TASK_RECEIVER;

FRTTransceiver_QueueHandle QUEUE_TO_RECEIVER;

FRTTransceiver_SemaphoreHandle SEMAPHORE1;


#define QUEUELENGTH  (3u)