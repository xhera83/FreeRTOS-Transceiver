#include <stdio.h>
#include "esp_system.h"
#include "FRTTransceiver.h"
#include "string.h"


using namespace FRTT;

FRTTTaskHandle TASK1_HANDLE;
FRTTTaskHandle TASK2_HANDLE;
FRTTQueueHandle QUEUE;
FRTTSemaphoreHandle SMPH;

#define QUEUELENGTH 3

#define DYNAMIC_STRUCTARRAY 0

#define INTDATA 0
#define STRUCTDATA 1
#define STDSTRINGDATA 0

struct TestStruct {
   int a = 99999;
   std::string s;
};

void dataAllocator(const FRTTDataContainerOnQueue & orig, FRTTTempDataContainer & temp)
{
   temp.data = orig.data;
   temp.senderAddress = orig.senderAddress;
   temp.u32AdditionalData = orig.u32AdditionalData;
   temp.u8DataType = orig.u8DataType;
}

void dataDestroyer(FRTTTempDataContainer & temp)
{
   // do nothing
}

/* Receiving task */
void TASK1(void *)
{  
   while(TASK1_HANDLE == nullptr || TASK2_HANDLE == nullptr) vTaskDelay(pdMS_TO_TICKS(15));

   #if (DYNAMIC_STRUCTARRAY == 1)
   FRTTransceiver comm(TASK1_HANDLE,1);         
   #else
	FRTTCommunicationPartner commStruct[1];
   FRTTransceiver comm(TASK1_HANDLE,&commStruct[0],1);
   #endif

   comm.addDataAllocateCallback(dataAllocator);
   comm.addDataFreeCallback(dataDestroyer);
   comm.addCommPartner(TASK2_HANDLE,QUEUE,QUEUELENGTH,SMPH,nullptr,0,nullptr,"TEST-2");

   /* sync with other task */
   comm.NotifyBasic(TASK2_HANDLE);

   bool res = true;

   while(res == true)
   {
      res = comm.readFromQueue(TASK2_HANDLE,eMultiSenderQueue::eNOMULTIQSELECTED,true,10,10);
   }

   const FRTTTempDataContainer * t1;
   const FRTTTempDataContainer * t2;
   const FRTTTempDataContainer * t3;

   int amountOfData = comm.bufferedDataFrom(TASK2_HANDLE,eMultiSenderQueue::eNOMULTIQSELECTED,true);

   if(amountOfData == 3)
   {
      printf("\nAmount of data received: %d\n",amountOfData);

      t1 = comm.getBufferedDataFrom(TASK2_HANDLE,eMultiSenderQueue::eNOMULTIQSELECTED,true,0); /* Oldest data --> INT    */
      t2 = comm.getBufferedDataFrom(TASK2_HANDLE,eMultiSenderQueue::eNOMULTIQSELECTED,true,1); /* STD::STRING            */
      t3 = comm.getBufferedDataFrom(TASK2_HANDLE,eMultiSenderQueue::eNOMULTIQSELECTED,true,2); /* Newest data --> STRUCT */

      printf("Data received from %p [INT]: %d\n",t1->senderAddress,*static_cast<int *>(t1->data));
      printf("Data received from %p [STD::STRING]: %s\n",t2->senderAddress,static_cast<std::string *>(t2->data)->c_str());
      printf("Data received from %p [TESTSTRUCT]: %d & %s\n\n\n",t3->senderAddress,static_cast<struct TestStruct *>(t3->data)->a,static_cast<struct TestStruct *>(t3->data)->s.c_str());
   }


   #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
   comm.printCommunicationsSummary();
   #endif

   vTaskDelete(nullptr);
}

/* Sender */
void TASK2(void * pvParams)
{  
   while(TASK1_HANDLE == nullptr || TASK2_HANDLE == nullptr) vTaskDelay(pdMS_TO_TICKS(15));

   #if (DYNAMIC_STRUCTARRAY == 1)
   FRTTransceiver comm(TASK2_HANDLE,1);         
   #else
	FRTTCommunicationPartner commStruct[1];
   FRTTransceiver comm(TASK2_HANDLE,&commStruct[0],1);
   #endif
   
   comm.addDataAllocateCallback(dataAllocator);
   comm.addDataFreeCallback(dataDestroyer);

   comm.addCommPartner(TASK1_HANDLE,nullptr,0,nullptr,QUEUE,QUEUELENGTH,SMPH,"TEST-2");
	
   /* sync with other task */
   while(comm.NotifyReceiveBasic(eFRTTNotifyActions::e_CLEARCOUNTONEXIT,1).getNotificationVal() == 0);
   
   int data = 15;
   std::string s("Im coming from TASK2");
   struct TestStruct testStruct;
   testStruct.s = "Im coming from TASK2";
   uint32_t u32AdditionalData = 0;


   
   comm.writeToQueue(TASK1_HANDLE,INTDATA,&data,200,200,++u32AdditionalData);
   vTaskDelay(pdMS_TO_TICKS(1)); /* ESP8266 has only one core!*/
   comm.writeToQueue(TASK1_HANDLE,STDSTRINGDATA,&s,200,200,++u32AdditionalData);
   vTaskDelay(pdMS_TO_TICKS(1)); /* ESP8266 has only one core!*/
   comm.writeToQueue(TASK1_HANDLE,STRUCTDATA,&testStruct,200,200,++u32AdditionalData);

   vTaskDelay(pdMS_TO_TICKS(4000));

   #ifdef FRTTRANSCEIVER_ANALYTICS_ENABLE
   comm.printCommunicationsSummary();
   #endif

   vTaskDelete(NULL);
}


/* app_main() like loop() */
#ifdef __cplusplus
extern "C"{
#endif  
void app_main()
{
   
   QUEUE = FRTTCreateQueue(QUEUELENGTH);
   SMPH = FRTTCreateSemaphore();

   FRTTCreateTask(TASK2,"test2-task",5000,nullptr,9,&TASK2_HANDLE);
   FRTTCreateTask(TASK1,"test1-task",5000,nullptr,9,&TASK1_HANDLE);

   vTaskDelay(pdMS_TO_TICKS(4000));
   esp_restart();
}
#ifdef __cplusplus
}
#endif 
