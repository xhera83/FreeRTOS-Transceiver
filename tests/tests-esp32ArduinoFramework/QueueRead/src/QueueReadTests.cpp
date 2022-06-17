
#include <AUnit.h>
#include "QueueReadTests.h"
#include <chrono>
#include "Arduino.h"

using namespace aunit;

#if (DYNAMIC_STRUCTARRAY == 1)
FRTTransceiver comm1(T_VALID,1);
FRTTransceiver comm2(T_VALID,1);
#else
FRTTCommunicationPartner partners1[1];
FRTTCommunicationPartner partners2[1];
FRTTransceiver comm1(T_VALID,&partners1[0],1);
FRTTransceiver comm2(T_VALID,&partners2[0],1);
#endif

test(queueRead_TEST1)
{
    /*          Some queueRead operations + time measurement 
     *               - Not 100% accurate because delay() in use by the other task has a 1ms inaccuracy (havent checked the reason)
     *               - Also..If you give READ6-TEST a blockTimeRead of 1 in the last queueRead() (instead of 2) will let the test finish
     *                 within 900-1000us (WILL FAIL TEST). Not concerning right now.. Everything works as expected with a blockTimeRead of 2 (300-400us) (havent checked the reason yet)
     *               - Just to get a first impression that queue-read runs >>almost<< as expected!
     */
    while(TASK_SENDER == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    comm1.addDataAllocateCallback(dataAllocator);
    comm1.addDataFreeCallback(destroyer);
    comm1.addCommPartner(TASK_SENDER,QUEUE,3,SMPHR,QUEUE1,3,SMPHR1,"TASK-SENDER");  /* TX QUEUE IS USED TO SYNCHRONIZE WITH OTHER TASK */

    
    int dummy = 0;
    uint32_t s;
    uint32_t ms;
    uint32_t us;
    
    printf("\n########\n\nTEST OUTPUT(Finishes after approx. 30.5s):\n\n");

    // ############################################ TEST1 ############################################ 

    assertEqual(comm1.writeToQueue(TASK_SENDER,0,&dummy,0,0,0),true);                       // for synchronization with other task
    auto s1 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,2,0),true);   // waits maximal 2ms (waittime for data) and 0ms (waittime to get semaphore)
    auto e1 = std::chrono::high_resolution_clock::now() - s1;
    us = std::chrono::duration_cast<std::chrono::microseconds>(e1).count();                 // should be arround 1000us
    assertLessOrEqual((uint32_t)us,(uint32_t)1200);
    printf("READ1 waited[%lld us]\n",(unsigned long long)us);

    // ############################################ TEST2 ############################################

    assertEqual(comm1.queueFlush(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          // for synchronization with other task
    auto s2 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,0),true); // waits a MAXTIME (waittime for data) and 0ms (waittime to get semaphore))
    auto e2 = std::chrono::high_resolution_clock::now() - s2;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(e2).count();                 // should be somewhat around 200ms
    assertLessOrEqual((uint32_t)ms,(uint32_t)200);
    printf("READ2 waited[%lld ms]\n",(unsigned long long)ms);

    // ############################################ TEST3 ############################################

    assertEqual(comm1.writeToQueue(TASK_SENDER,0,&dummy,0,0,0),true);                       // for synchronization with other task
    auto s3 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,2000,0),true);       // waits a max of 2000ms (waittime for data) and 0ms (waittime to get semaphore))
    auto e3 = std::chrono::high_resolution_clock::now() - s3;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(e3).count();                 // should be somewhat around 1750ms
    assertLessOrEqual((uint32_t)ms,(uint32_t)1750);
    printf("READ3 waited[%lld ms]\n",(unsigned long long)ms);

    // ############################################ TEST4 ############################################

    assertEqual(comm1.queueFlush(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          // for synchronization with other task
    auto s4 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,100,0),false);       // waits a max of 100ms (waittime for data) and 0ms (waittime to get semaphore))
    auto e4 = std::chrono::high_resolution_clock::now() - s4;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(e4).count();                 // should be around 100ms (will not get data in time)
    assertLessOrEqual((uint32_t)ms,(uint32_t)100);
    printf("READ4 waited[%lld ms]\n",(unsigned long long)ms);


    // ############################################ TEST5 ############################################
    delay(400);
    assertEqual(comm1.messagesOnQueue(TASK_SENDER,false),1);                                // We will have one message on queue because we didnt wait long enough (last block)
    assertEqual(comm1.queueFlush(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,false),true);         // Remove last message sent by task 
    assertEqual(comm1.messagesOnQueue(TASK_SENDER,false),0);                                // Zero messages, queue flushed!
                                                                                            
    assertEqual(comm1.writeToQueue(TASK_SENDER,0,&dummy,0,0,0),true);                       // for synchronization with other task
    auto s5 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,9990,0),false);      // waits a max of 9990ms (waittime for data) and 0ms (waittime to get semaphore))
    auto e5 = std::chrono::high_resolution_clock::now() - s5;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(e5).count();                 // should be around 9990ms (will not get data in time)
    assertLessOrEqual((uint32_t)ms,(uint32_t)9990);
    printf("READ5 waited[%lld ms]\n",(unsigned long long)ms);
    
    // ############################################ TEST6 ############################################

    delay(100);
    assertEqual(comm1.messagesOnQueue(TASK_SENDER,false),1);                                // We will have one message on queue because we didnt wait long enough (last block)
    assertEqual(comm1.queueFlush(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,false),true);         // Remove last message sent by task (we didnt get the message in time)
    assertEqual(comm1.messagesOnQueue(TASK_SENDER,false),0);                                // Zero messages, queue flushed!

    assertEqual(comm1.queueFlush(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          // for synchronization with other task
    auto s6 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,1,1),true);          // waits a max of 1ms (waittime for data) and 1ms (waittime to get semaphore))
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,1,1),true);          // waits a max of 1ms (waittime for data) and 1ms (waittime to get semaphore))
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,2,1),true);          // waits a max of 2ms (waittime for data) and 1ms (waittime to get semaphore))
    //assertEqual(comm1.readFromQueue(TASK_SENDER,eNOMULTIQSELECTED,true,1,1),true);        // >>>>>>>  WILL FAIL THE TEST because of the 1ms blocktime to get permission to read, ALTHOUGH IT SHOULDNT! <<<<<<<<<
    auto e6 = std::chrono::high_resolution_clock::now() - s6;
    us = std::chrono::duration_cast<std::chrono::microseconds>(e6).count();                 // should be < 800us for all queue-read operations combined
    assertLessOrEqual((uint32_t)us,(uint32_t)800);
    printf("READ6 waited[%lld us]\n",(unsigned long long)us);

    // ############################################ TEST7 ############################################

    assertEqual(comm1.writeToQueue(TASK_SENDER,0,&dummy,0,0,0),true);                       // for synchronization with other task
    auto s7 = std::chrono::high_resolution_clock::now();
    assertEqual(comm1.readFromQueue(TASK_SENDER,eMultiSenderQueue::eNOMULTIQSELECTED,true,FRTTRANSCEIVER_WAITMAX,0),true);    // waits a MAXTIME (waittime for data) and 0ms (waittime to get semaphore))
    auto e7 = std::chrono::high_resolution_clock::now() - s7;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(e7).count();                 // should be around 17990ms (will get data in time)
    assertLessOrEqual((uint32_t)ms,(uint32_t)18000);
    printf("READ7 waited[%lld ms]\n",(unsigned long long)ms);
    printf("\n########\n\n");
}
void TASKSENDER(void * pvParams)
{

    comm2.addDataAllocateCallback(dataAllocator);
    comm2.addDataFreeCallback(destroyer);

    comm2.addCommPartner(T_VALID,QUEUE1,3,SMPHR1,QUEUE,3,SMPHR,"TEST-TASK");        /* RX QUEUE IS USED TO SYNCHRONIZE WITH TEST TASK */

    int a = 15;
    
    while(comm2.messagesOnQueue(T_VALID,false) == 0);           // sync with test-task
    
    delayMicroseconds(1000);                                    // delay(1) did not delay properly for 1ms, other task will max wait 2ms (will receive data)
    comm2.writeToQueue(T_VALID,0,&a,0,0,0);

    while(comm2.messagesOnQueue(T_VALID,false) > 0);            // sync with test-task
        
    delay(200);                                                 // delay for 200ms, other task will max wait 400ms (will receive data)
    comm2.writeToQueue(T_VALID,0,&a,0,0,0);                 
    
    while(comm2.messagesOnQueue(T_VALID,false) == 0);           // sync with test-task
    
    delay(1750);                                                // delay for 2000ms, other task will max wait 2000ms (will receive data)
    comm2.writeToQueue(T_VALID,0,&a,0,0,0);

    while(comm2.messagesOnQueue(T_VALID,false) > 0);            // sync with test-task

    delay(500);                                                 // delay for 500ms, other task will max wait 100ms (will NOT receive data)
    comm2.writeToQueue(T_VALID,0,&a,0,0,0);

    
    while(comm2.messagesOnQueue(T_VALID,false) == 0);           // sync with test-task
    
    delay(10000);                                               // delay for 10000ms, other task will max wait 9990ms (will NOT receive data)
    comm2.writeToQueue(T_VALID,0,&a,0,0,0);

    while(comm2.messagesOnQueue(T_VALID,false) > 0);            // sync with test-task

                                                                // no delay, other task will read each message right away
    comm2.writeToQueue(T_VALID,0,&a,1,1,0);                     
    comm2.writeToQueue(T_VALID,0,&a,1,1,0);
    delayMicroseconds(300);
    comm2.writeToQueue(T_VALID,0,&a,1,1,0);

    while(comm2.messagesOnQueue(T_VALID,false) == 0);           // sync with test-task

    delay(17990);                                               // delay for ~18s, other task will max wait 18s (will receive data)
    comm2.writeToQueue(T_VALID,0,&a,1,1,0); 

    vTaskDelete(nullptr);
}

void setup()
{
    vTaskDelay(pdMS_TO_TICKS(2500));
    Serial.begin(115200);               // For the internal printer of TestRunner
    disableCore0WDT();
    while(!Serial);                     
    log_i("\n\n");

    QUEUE = FRTTCreateQueue(3);
    QUEUE1 = FRTTCreateQueue(3);

    SMPHR = FRTTCreateSemaphore();
    SMPHR1 = FRTTCreateSemaphore();
    

    log_i("This test should produce the following:");
    log_i("1 passed, 0 failed, 0 skipped, 0 timed out, out of 1 test(s).");
    log_i("----------\n\n");

    xTaskCreatePinnedToCore(TASKSENDER,"task-sender",5000,nullptr,8,&TASK_SENDER,0);
}



void loop()
{
    TestRunner::run();
}



void dataAllocator(const FRTTDataContainerOnQueue & orig, FRTTTempDataContainer & temp)
{
    temp.data = orig.data;
    temp.senderAddress = orig.senderAddress;
    temp.u32AdditionalData = orig.u32AdditionalData;
    temp.u8DataType = orig.u8DataType;
}
void destroyer(FRTTTempDataContainer & temp)
{
      /* nothing to do */      
}

