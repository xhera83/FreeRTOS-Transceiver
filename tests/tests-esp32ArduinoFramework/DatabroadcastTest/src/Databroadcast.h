#include "FRTTransceiver.h"

using namespace FRTT;

#define DYNAMIC_STRUCTARRAY 0

#define TASK_INVALID        (nullptr)
#define TASK_VALID          (0x1)

#define QUEUE_INVALID       (nullptr)
#define QUEUE_VALID         (0x1)

#define SEMPHR_INVALID      (nullptr)
#define SEMPHR_VALID        (0x1)

#define PARTNERNAME         ("ABC")
#define PARTNERNAME_EMPTY   ("")

FRTTTaskHandle T_INVALID = TASK_INVALID;
FRTTTaskHandle T_VALID = (FRTTTaskHandle) 0x1;
FRTTTaskHandle T_VALID2 = (FRTTTaskHandle) 0x2;
FRTTTaskHandle T_VALID3 = (FRTTTaskHandle) 0x3;
FRTTTaskHandle T_VALID4 = (FRTTTaskHandle) 0x4;
FRTTTaskHandle T_VALID5 = (FRTTTaskHandle) 0x5;
FRTTTaskHandle T_VALID6 = (FRTTTaskHandle) 0x6;

FRTTQueueHandle Q_INVALID = QUEUE_INVALID;
FRTTQueueHandle Q_VALID;
FRTTQueueHandle Q_VALID2;
FRTTQueueHandle Q_VALID3;
FRTTQueueHandle Q_VALID4;
FRTTQueueHandle Q_VALID5;
FRTTQueueHandle Q_VALID6;
FRTTQueueHandle Q_VALID7;
FRTTQueueHandle Q_VALID8;
FRTTQueueHandle Q_VALID9;

FRTTSemaphoreHandle S_INVALID = SEMPHR_INVALID;
FRTTSemaphoreHandle S_VALID;
FRTTSemaphoreHandle S_VALID2;
FRTTSemaphoreHandle S_VALID3;
FRTTSemaphoreHandle S_VALID4; 
FRTTSemaphoreHandle S_VALID5;
FRTTSemaphoreHandle S_VALID6;
FRTTSemaphoreHandle S_VALID7;
FRTTSemaphoreHandle S_VALID8;
FRTTSemaphoreHandle S_VALID9;

void dataAllocator(const FRTTDataContainerOnQueue & orig, FRTTTempDataContainer & temp);
void destroyer(FRTTTempDataContainer & temp);
