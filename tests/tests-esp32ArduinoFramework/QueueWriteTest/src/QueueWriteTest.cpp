
#include <AUnit.h>
#include "QueueWriteTest.h"

using namespace aunit;

#if (DYNAMIC_STRUCTARRAY == 1)
FRTTransceiver commTest(T_VALID,3);
FRTTransceiver comm(TASK_TESTER1,1);
#else
FRTTCommunicationPartner partners[3];
FRTTransceiver commTest(T_VALID,&partners[0],3);

FRTTCommunicationPartner partners2[1];

FRTTransceiver comm(TASK_TESTER1,&partners2[0],1);
#endif 
class QueueWriteTest : public TestOnce {
    
    

    protected:
        void setup() override {
            TestOnce::setup();
            while(TASK_TESTER1 == nullptr) vTaskDelay(pdMS_TO_TICKS(1));
        }
        void doFailedQueueWrites()
        {   
            

            assertEqual(commTest.writeToQueue(T_VALID,0,nullptr,-2,-2,0),false);   /* will return 'false', because no callback functions inserted */

            commTest.addDataAllocateCallback(dataAllocator);
            commTest.addDataFreeCallback(destroyer);

            assertEqual(commTest.writeToQueue(T_VALID,0,nullptr,-2,-2,0),false);                    /* will return 'false', because T_VALID can not be found in the list of communications */

            assertEqual(commTest.addCommPartner(T_VALID,nullptr,1,nullptr,nullptr,1,nullptr),true); /* Will return 'true', because (see AddCommPartnerTest.cpp)*/
            assertEqual(commTest.writeToQueue(T_VALID,0,nullptr,-2,-2,0),false);                    /* will return 'false' because there are no semaphores for the tx line 
                                                                                                        (also not queues but semaphores checked first)*/

            assertEqual(commTest.addCommPartner(T_VALID2,nullptr,0,nullptr,Q_VALID,1,S_VALID),true);   /* Will return 'true', because (see AddCommPartnerTest.cpp)*/
            assertEqual(commTest.writeToQueue(T_VALID2,0,nullptr,-2,-2,0),false);                      /* Will return 'false' because data == NULL */
            assertEqual(commTest.writeToQueue(T_VALID2,0,&DATA,-2,-2,0),false);                        /* Will return 'false' because block time's are invalid */
            assertEqual(commTest.readFromQueue(T_VALID2,eMultiSenderQueue::eNOMULTIQSELECTED,true,100,100),false); /* Will return false because we have no rx queue */

            /* From here on we'll not work with these added communication lines because they contain pseudo queue's/partner addresses and the next tests would contain actual write operations */
        }
        void doSetup()
        {
            /* From here on 1 communication line left to add */
            assertEqual(commTest.addCommPartner(TASK_TESTER1,nullptr,0,nullptr,QUEUE_TO_TESTER1,1,SMPHR_TESTER1,"TESTER1"),true); /* Will return 'true', because (see AddCommPartnerTest.cpp)*/
        }
        void writeToQueue()
        {
            assertEqual(commTest.writeToQueue(TASK_TESTER1,0,&DATA,0,0,0),true);  /* Will return 'true', because there will be space left on the queue (no waittime needed)*/
            /**
             *  Will return 'false', because the queue is full (caused by line above) and the receiving task wont make a read operation in time (this task only waits for 100ms)
             */
            assertEqual(commTest.writeToQueue(TASK_TESTER1,0,&DATA,100,100,0),false);
            assertEqual(commTest.writeToQueue(TASK_TESTER1,0,&DATA,200,200,0),true); /* Will return 'true' because the receiving task read the first element (we waited < 300ms) */
        }

        /* The other tasks only has a maximum of 20ms delay (wait for permission to flush queue (10ms) + wait for next check if messages available(10ms) )*/
        void fasterTransmit()
        {
            bool res = false;
            for(uint8_t u8I = 0;u8I < 10;u8I++)
            {   
                res = commTest.writeToQueue(TASK_TESTER1,0,&DATA,30,30,0);
                assertEqual(res,true);
            }
        }

        void hyperTransmit()
        {   
            /* To wait for the other task (because the other task uses a 10ms delay between queue flush operations)*/
            while(commTest.messagesOnQueue(TASK_TESTER1,true) > 0);
            bool res;
            for(uint8_t u8I = 0;u8I < 10;u8I++)
            {
                assertEqual(res = commTest.writeToQueue(TASK_TESTER1,0,&DATA,3,3,0),true);
            }
        }
        int DATA = 10;
};

testF(QueueWriteTest,queueWriteTest)
{   
    /* assertNoFatalFailure() stops this test if at least one assertXxx inside QueueWriteTest fails */
    
    /* Multiple attempts to write to a queue with an incorrect setup */
    assertNoFatalFailure(doFailedQueueWrites());

    /* Setup of correct connections to the TESTER_X tasks */
    assertNoFatalFailure(doSetup());

    /* has 1 failed write operation, 2 successful ones*/
    assertNoFatalFailure(writeToQueue());

    /* Faster transmission of data with less delay (other task) and little wait time (this and other task)*/
    assertNoFatalFailure(fasterTransmit());

    /* Faster transmission of data with no delay (this and other task) and very little wait time (this and other task)*/
    assertNoFatalFailure(hyperTransmit());
}

void setup()
{
    vTaskDelay(pdMS_TO_TICKS(2500));
    Serial.begin(115200);               // For the internal printer of TestRunner
    while(!Serial);                     
    log_i("\n\n");
    
    TestRunner::setTimeout(0);          // Let the tests run as long as possible (~18Hrs)

    QUEUE_TO_TESTER1 = FRTTCreateQueue(1);

    SMPHR_TESTER1 = FRTTCreateSemaphore();
    

    log_i("This test should produce the following:");
    log_i("1 passed, 0 failed, 0 skipped, 0 timed out, out of 1 test(s).");
    log_i("----------\n\n");

    xTaskCreatePinnedToCore(TESTER1,"TESTER1",5000u,nullptr,6,&TASK_TESTER1,0);
}


void loop()
{
    TestRunner::run();
} 

void TESTER1(void *){
    
    comm.addDataAllocateCallback(dataAllocator);
    comm.addDataFreeCallback(destroyer);

    comm.addCommPartner(T_VALID,QUEUE_TO_TESTER1,1,SMPHR_TESTER1,nullptr,0,nullptr,"MASTER-TESTER");

    while(comm.messagesOnQueue(T_VALID,false) == 0);
    vTaskDelay(pdMS_TO_TICKS(150));
    comm.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,400,false);

    while(comm.messagesOnQueue(T_VALID,false) == 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    comm.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,400,false);

    for(uint8_t u8I = 0; u8I < 10;u8I++)
    {   
        vTaskDelay(pdMS_TO_TICKS(10));
        if(comm.messagesOnQueue(T_VALID,false) > 0)
        {
            comm.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,10,false);
        }
    }
    
    for(uint8_t u8I = 0; u8I < 10;u8I++)
    {
        if(comm.messagesOnQueue(T_VALID,false) > 0)
        {
            comm.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,false);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    } 
    
    vTaskDelete(nullptr);
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
            
}