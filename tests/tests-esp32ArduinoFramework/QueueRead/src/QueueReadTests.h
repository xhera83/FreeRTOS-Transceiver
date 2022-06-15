#include "FRTTransceiver.h"

using namespace FRTT;

#define DYNAMIC_STRUCTARRAY 1

#define TASK_INVALID        (nullptr)
#define TASK_VALID          (0x1)

#define QUEUE_INVALID       (nullptr)
#define QUEUE_VALID         (0x1)

#define SEMPHR_INVALID      (nullptr)
#define SEMPHR_VALID        (0x1)

#define PARTNERNAME         ("ABC")
#define PARTNERNAME_EMPTY   ("")

FRTTTaskHandle TASK_SENDER;

FRTTQueueHandle QUEUE;
FRTTQueueHandle QUEUE1;

FRTTSemaphoreHandle SMPHR;
FRTTSemaphoreHandle SMPHR1;


FRTTTaskHandle T_INVALID = TASK_INVALID;
FRTTTaskHandle T_VALID = (FRTTTaskHandle) 0x1;

FRTTSemaphoreHandle S_INVALID = SEMPHR_INVALID;
FRTTSemaphoreHandle S_VALID = (FRTTSemaphoreHandle) 0x1;

FRTTQueueHandle Q_INVALID = QUEUE_INVALID;
FRTTQueueHandle Q_VALID = (FRTTQueueHandle) 0x1;

void dataAllocator(const FRTTDataContainerOnQueue & orig, FRTTTempDataContainer & temp);
void destroyer(FRTTTempDataContainer & temp);

void TASKSENDER(void * pvParams);
