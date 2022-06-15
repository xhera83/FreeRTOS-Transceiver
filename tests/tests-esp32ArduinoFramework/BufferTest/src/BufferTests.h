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

enum class Datatypes : uint8_t
{
	eINTPTR = 0,
	eCHARPTR,
	eSTRINGPTR,
	eRANDOM1,
	eRANDOM2,
	eRANDOM3,
};

FRTTQueueHandle QUEUE;
FRTTQueueHandle QUEUE2;
FRTTQueueHandle QUEUE3;
FRTTQueueHandle QUEUE4;

FRTTQueueHandle QUEUE5;
FRTTQueueHandle QUEUE6;
FRTTQueueHandle QUEUE7;

FRTTSemaphoreHandle SMPHR;
FRTTSemaphoreHandle SMPHR2;
FRTTSemaphoreHandle SMPHR3;
FRTTSemaphoreHandle SMPHR4;

FRTTSemaphoreHandle SMPHR5;
FRTTSemaphoreHandle SMPHR6;
FRTTSemaphoreHandle SMPHR7;

FRTTTaskHandle T_INVALID = TASK_INVALID;
FRTTTaskHandle T_VALID = (FRTTTaskHandle) 0x1;
FRTTTaskHandle T_VALID2 = (FRTTTaskHandle) 0x2;
FRTTTaskHandle T_VALID3 = (FRTTTaskHandle) 0x3;
FRTTTaskHandle T_VALID4 = (FRTTTaskHandle) 0x4;
FRTTTaskHandle T_VALID5 = (FRTTTaskHandle) 0x5;
FRTTTaskHandle T_VALID6 = (FRTTTaskHandle) 0x6;

FRTTQueueHandle Q_INVALID = QUEUE_INVALID;
FRTTQueueHandle Q_VALID = (FRTTQueueHandle) 0x1;
FRTTQueueHandle Q_VALID2 = (FRTTQueueHandle) 0x2;
FRTTQueueHandle Q_VALID3 = (FRTTQueueHandle) 0x3;
FRTTQueueHandle Q_VALID4 = (FRTTQueueHandle) 0x4;
FRTTQueueHandle Q_VALID5 = (FRTTQueueHandle) 0x5;
FRTTQueueHandle Q_VALID6 = (FRTTQueueHandle) 0x6;

FRTTSemaphoreHandle S_INVALID = SEMPHR_INVALID;
FRTTSemaphoreHandle S_VALID = (FRTTSemaphoreHandle) 0x1;
FRTTSemaphoreHandle S_VALID2 = (FRTTSemaphoreHandle) 0x2;
FRTTSemaphoreHandle S_VALID3 = (FRTTSemaphoreHandle) 0x3;
FRTTSemaphoreHandle S_VALID4 = (FRTTSemaphoreHandle) 0x4;
FRTTSemaphoreHandle S_VALID5 = (FRTTSemaphoreHandle) 0x5;
FRTTSemaphoreHandle S_VALID6 = (FRTTSemaphoreHandle) 0x6;

void dataAllocator(const FRTTDataContainerOnQueue & orig, FRTTTempDataContainer & temp);
void destroyer(FRTTTempDataContainer & temp);
