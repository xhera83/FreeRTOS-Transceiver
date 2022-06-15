
#include <AUnit.h>
#include "Databroadcast.h"

using namespace aunit;

#if (DYNAMIC_STRUCTARRAY == 1)
FRTTransceiver comm1(T_VALID,5);
FRTTransceiver comm2(T_VALID,5);  
FRTTransceiver comm3(T_VALID,2);                
FRTTransceiver comm4(T_VALID,3);                
#else
FRTTCommunicationPartner partners1[5];
FRTTCommunicationPartner partners2[5];
FRTTCommunicationPartner partners3[2];
FRTTCommunicationPartner partners4[3];

FRTTransceiver comm1(T_VALID,&partners1[0],5);
FRTTransceiver comm2(T_VALID,&partners2[0],5);  
FRTTransceiver comm3(T_VALID,&partners3[0],2);                
FRTTransceiver comm4(T_VALID,&partners4[0],3);               
#endif

test(twoOutOfFourTxQueuesDB)
{   
    comm1.addDataAllocateCallback(dataAllocator);
    comm1.addDataFreeCallback(destroyer);

    uint8_t u8Data = 15;

    assertEqual(comm1.addCommPartner(T_VALID,nullptr,0,nullptr,Q_VALID,1,S_VALID,"1"),true);                    /* TX QUEUE ADDED */
    assertEqual(comm1.addCommPartner(T_VALID2,nullptr,0,nullptr,Q_VALID2,1,S_VALID2,"2"),true);                 /* TX QUEUE ADDED */

    assertEqual(comm1.addCommPartner(T_VALID3,Q_VALID3,1,S_VALID3,nullptr,0,nullptr,"RX-1"),true);              /* RX QUEUE ADDED */
    assertEqual(comm1.addCommPartner(T_VALID4,Q_VALID4,1,S_VALID4,nullptr,0,nullptr,"RX-2"),true);              /* RX QUEUE ADDED */

    assertEqual(comm1.addMultiSenderPartner(Q_VALID5,1,S_VALID5,"MULTI-1"),true);                               /* Multi-Sender-Queue added */
    assertEqual(comm1.addMultiSenderPartner(Q_VALID5,1,S_VALID6,"MULTI-2"),false);                              /* Multi-Sender-Queue adde, no connetions left, will fail */

    assertEqual(comm1.databroadcast(0,&u8Data,500,500,0),true);             /* broadcast to two tx-queues (500ms waittime for the write operation, 500ms waittime for semaphore) */
    /* broadcast towards two tx-queues (2ms waittime for the write operation, 2ms waittime for semaphore). Will fail since we already posted data to those queues*/
    assertEqual(comm1.databroadcast(0,&u8Data,2,2,0),false); 

    //comm1.printCommunicationsSummary();
}

test(noTxQueuesForDatabroadcast)
{
    comm2.addDataAllocateCallback(dataAllocator);
    comm2.addDataFreeCallback(destroyer);

    uint8_t u8Data = 15;

    assertEqual(comm2.addCommPartner(T_VALID,Q_VALID,1,S_VALID,nullptr,0,nullptr,"RX-1"),true);         /* RX QUEUE ADDED */
    assertEqual(comm2.addCommPartner(T_VALID2,Q_VALID2,1,S_VALID2,nullptr,0,nullptr,"RX-2"),true);      /* RX QUEUE ADDED */

    assertEqual(comm2.addCommPartner(T_VALID3,Q_VALID3,1,S_VALID3,nullptr,0,nullptr,"RX-3"),true);      /* RX QUEUE ADDED */
    assertEqual(comm2.addCommPartner(T_VALID4,Q_VALID4,1,S_VALID4,nullptr,0,nullptr,"RX-4"),true);      /* RX QUEUE ADDED */

    assertEqual(comm2.addMultiSenderPartner(Q_VALID5,1,S_VALID5,"MULTI-1"),true);                       /* Multi-Sender-Queue */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID6,1,S_VALID6,"MULTI-2"),false);                      /* Multi-Sender-Queue, no connections left, will fail */

    assertEqual(comm2.databroadcast(0,&u8Data,500,500,0),false);                                        /* Gonna fail, since we do not have a tx queue */
    assertEqual(comm2.databroadcast(0,&u8Data,2,2,0),false);                                            /* Gonna fail again, since we still do not have a tx queue */

    //comm2.printCommunicationsSummary();
}

test(oneOfTwoTxQueuesDB)
{
    comm3.addDataAllocateCallback(dataAllocator);
    comm3.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);
    FRTTTaskHandle TASK2 = (FRTTTaskHandle)(T_VALID + 0x6);

    uint8_t u8Data = 15;
    assertEqual(comm3.addCommPartner(T_VALID,nullptr,0,nullptr,Q_VALID6,1,S_VALID6,"TX-1"),true);          /* TX QUEUE ADDED */
    assertEqual(comm3.addCommPartner(T_VALID2,Q_VALID,1,S_VALID,nullptr,0,nullptr,"RX-1"),true);        /* RX QUEUE ADDED */

    assertEqual(comm3.databroadcast(0,&u8Data,500,500,0),true); /* One tx queue for databroadcasting available */
}

test(threeOfThreeTxQueuesDB)
{
    comm4.addDataAllocateCallback(dataAllocator);
    comm4.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);
    FRTTTaskHandle TASK2 = (FRTTTaskHandle)(T_VALID + 0x2);
    FRTTTaskHandle TASK3 = (FRTTTaskHandle)(T_VALID + 0x3);

    uint8_t u8Data = 15;
    
    assertEqual(comm4.addCommPartner(TASK1,nullptr,0,nullptr,Q_VALID7,1,S_VALID7,"1"),true);    /* TX QUEUE ADDED */
    assertEqual(comm4.addCommPartner(TASK2,nullptr,0,nullptr,Q_VALID8,1,S_VALID8,"2"),true);    /* TX QUEUE ADDED */
    assertEqual(comm4.addCommPartner(TASK3,nullptr,0,nullptr,Q_VALID9,1,S_VALID9,"3"),true);    /* TX QUEUE ADDED */

    assertEqual(comm4.databroadcast(0,&u8Data,500,500,0),true); /* 3 tx queues. everything must pass. */
}

void setup()
{
    vTaskDelay(pdMS_TO_TICKS(2500));
    Serial.begin(115200);               // For the internal printer of TestRunner
    while(!Serial);                     
    log_i("\n\n");
    
    Q_VALID = FRTTCreateQueue(1);
    Q_VALID2 = FRTTCreateQueue(1);
    Q_VALID3 = FRTTCreateQueue(1);
    Q_VALID4 = FRTTCreateQueue(1);
    Q_VALID5 = FRTTCreateQueue(1);
    Q_VALID6 = FRTTCreateQueue(1);
    Q_VALID7 = FRTTCreateQueue(1);
    Q_VALID8 = FRTTCreateQueue(1);
    Q_VALID9 = FRTTCreateQueue(1);

    S_VALID = FRTTCreateSemaphore();
    S_VALID2 = FRTTCreateSemaphore();
    S_VALID3 = FRTTCreateSemaphore();
    S_VALID4 = FRTTCreateSemaphore();
    S_VALID5 = FRTTCreateSemaphore();
    S_VALID6 = FRTTCreateSemaphore();
    S_VALID7 = FRTTCreateSemaphore();
    S_VALID8 = FRTTCreateSemaphore();
    S_VALID9 = FRTTCreateSemaphore();



    TestRunner::setTimeout(0);          // Let the tests run as long as possible (~18Hrs)

    log_i("This test should produce the following:");
    log_i("4 passed, 0 failed, 0 skipped, 0 timed out, out of 4 test(s).");
    log_i("----------\n\n");

    
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