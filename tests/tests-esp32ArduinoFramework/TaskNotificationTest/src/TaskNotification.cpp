
#include <AUnit.h>
#include "TaskNotificationTest.h"

using namespace aunit;

/**
 *              TEST1: 
 *                      - Tests if the methods fail when wrong parameters are provided 
 *                      - Does not 'notify' any task because we use self-build taskhandles
 *              TEST2:
 *                      - Tests all possible notify-receive methods
 *                      - We use a self-build owner task handle because freertos determines the task handle on its own
 *                      - We use a self-build taskhandle because we dont notify anyone. Sync-Queue is valid.
 *                      - Gets a simple notification from NotifierTask
 *                      - Synchronization over a queue
 *              TEST3:
 *                      - Tests all possible notify methods
 *                      - Uses an actual task handle
 * 
 * 
 */

#if (DYNAMIC_STRUCTARRAY == 1)
FRTTransceiver commTest1(T_VALID,1);
FRTTransceiver commTest23(T_VALID,1);

FRTTransceiver commNotifier(T_VALID,1);               
#else
FRTTCommunicationPartner partner[1];
FRTTCommunicationPartner partner2[1];
FRTTransceiver commTest1(T_VALID,&partner[0],1);
FRTTransceiver commTest23(T_VALID,&partner2[0],1);

FRTTCommunicationPartner partnerNotifier[1];
FRTTransceiver commNotifier(T_VALID,&partnerNotifier[0],1);
#endif

class NotificationTest : public TestOnce {
    protected : 
        void setup() override {
            TestOnce::setup();
        }

        void notifyWrongSetup_TEST1()
        {
            commTest1.addDataAllocateCallback(dataAllocator);
            commTest1.addDataFreeCallback(destroyer);

            /* Here we use self build task-handles and only test these methods until before the real 'notify' call happens (we test the actual calls and actions in TEST3) */

            assertEqual(commTest1.addCommPartner(T_VALID,nullptr,0,nullptr,nullptr,0,nullptr,"xyz"),true);

            assertEqual(commTest1.NotifyBasic(nullptr),false);                                                              /* notify nullptr err, rest can not be tested here*/
            assertEqual(commTest1.NotifyBasic(T_VALID5),false);                                                             /* We do not have this task in our list */

            assertEqual(commTest1.NotifyExtended(nullptr,eFRTTNotifyActions::e_NoAction,15),false);                         /* notify nullptr eerr */
            assertEqual(commTest1.NotifyExtended(T_VALID5,eFRTTNotifyActions::e_NoAction,15),false);                        /* We do not have this task in our list */

            assertEqual(commTest1.NotifyExtended(T_VALID,eFRTTNotifyActions::e_CLEARCOUNTONEXIT,15),false);                 /* Wrong action */
            assertEqual(commTest1.NotifyExtended(T_VALID,eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT,15),false);              /* Wrong action */

            /* Receive methods must be tested in a different way because they return a FRTTranceiver obj! */
        }

        void notifyReceiveBasic_TEST2()
        {
            commTest23.addDataAllocateCallback(dataAllocator);
            commTest23.addDataFreeCallback(destroyer);

            assertEqual(commTest23.addCommPartner(T_VALID,nullptr,0,nullptr,QSYNC_TEST2_AND3,3,SMPHR_TEST2_AND3,"SYNC-TO-NOTIFIER"),true);

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,0,0,0),true);                                  /* sync with notifier */ 
            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_CLEARCOUNTONEXIT,500).getNotificationVal(),(unsigned long)1); /* other tasks increments our notif.value by one, we clear it to 0 */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);  /* sync with notifier */
            
            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT,500).getNotificationVal(),(unsigned long)1); /* other tasks increments our notif.value by one, we decrement it to 0 */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,0,0,0),true);                                  /* sync with notifier */

            vTaskDelay(pdMS_TO_TICKS(1)); /* Give the Notifier task a bit of time to increment to 6! */

            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT,500).getNotificationVal(),(unsigned long)6); /* should be 6 because NotiferTask will do 6x FRTT::FRTTransceiver::NotifyBasic()*/
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);                                               /* sync with notifier */

            vTaskDelay(pdMS_TO_TICKS(1)); /* Give the Notifier task a bit of time to increment from 5 to 10! */

            /* should be 10 because NotiferTask will do 5x increment with FRTT::FRTTransceiver::NotifyBasic() and our value was 5 before (we decremented from 6 to 5 in the last call to FRTT::FRTTransceiver::NotifyReceiveBasic())*/
            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_CLEARCOUNTONEXIT,500).getNotificationVal(),(unsigned long)10);
            assertEqual(commTest23.hasNotification(),true);

            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_CLEARCOUNTONEXIT,50).getNotificationVal(),(unsigned long)0); /* The other task does not notify again. --> Value will be 0 */
            assertEqual(commTest23.hasNotification(),false);                                                                                            /* hasNotification() must be false*/

            /* THE TEST WITH THE EXTENDED FUNCTIONALITY WILL BE IN TEST_3 */
        }

        void notifyReceiveExtended_TEST3()
        {
            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */

            assertEqual(commTest23.NotifyReceiveExtended(0,0,500).hasNotification(),true);                              /* notifier will only set our notification state to pending, value will be zero */
            assertEqual((unsigned long)commTest23.getNotificationVal(),(unsigned long)0);                               

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x8,0x4,500).getNotificationVal(),0xCUL);       /* 
                                                                                                                            We must receive 0xC == b1100, 0x8 clearMask wont be used, because notification pending.
                                                                                                                            0x4 clearMask will be used so that we leave our task notification value with: b1000 
                                                                                                                            Will check the correctness in the next NotifyReceiveExtended()
                                                                                                                        */
            assertEqual(commTest23.hasNotification(),true);                                                                

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x8,500).getNotificationVal(),0x8UL);       /* 
                                                                                                                            NotifierTask will only set our notif. state to pending. 0x8 should be left from the last call,
                                                                                                                            because we used 0x4 clearMaskOnExit. 0x8 clearMaskOnExit will set our notif. val to 0x0,
                                                                                                                            which will be checked below
            
                                                                                                                        */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x0,500).getNotificationVal(),0x0UL);       /* Must be 0x0 because we cleared it on exit last time */
            assertEqual(commTest23.hasNotification(),true);                                                             

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x0,500).getNotificationVal(),0xCUL);       /* Again 0xC == b1100, we do not clear anything */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x4,0x0,500).getNotificationVal(),0x8UL);       /* 
                                                                                                                            We use the clearMaskOnEntry and ultimately set our notification value to b1000
                                                                                                                            The notifier has a small delay so we cann apply the mask
                                                                                                                        */

            assertEqual(commTest23.hasNotification(),true);
            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x8,2).getNotificationVal(),0x8UL);         /*  We will receive nothing but try to clear the notification val
                                                                                                                            on exit (clearBits(0x8,0x8) == 0x0)! Shouldnt work! */     
            assertEqual(commTest23.hasNotification(),false);                                                            /*  We didnt receive anything in the last call ---> should be set to false internally */

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x9,2).getNotificationVal(),0x9UL);         /* NotifierTask will simply increment our notificaiton value (which was 0x8 because we couldnt clear 0x8)
                                                                                                                           We will clear 0x9 on exit ...                                                                                                           
                                                                                                                        */

            assertEqual(commTest23.hasNotification(),true);   

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          /* sync with notifier */

            /* Here notfier task will first set our notification value to 0x1, and then try to use eFRTTNotifyActions_eSetValueWithOverwrite with value 0x2 even though a notification is already pending */
            delayMicroseconds(400); /* Give him time to do both notifications */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x2,2).getNotificationVal(),0x2UL);         /* We should receive the latter value */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0xF1F1F1F1,2).getNotificationVal(),0xF1F1F1F1UL);  /* 
                                                                                                                                    Notifier uses  eFRTTNotifyActions_eSetBits with this mask(16bit):

                                                                                                                                    (0xF1F1F1F1 = 1111 0001 1111 0001 1111 0001 1111 0001, )
            
                                                                                                                                */
        }

        void notifyReceiveMixed_TEST4()
        {
            /* we arrive here with an internal notification value of 0x0 (we cleared 0xF1F1F1F1 onExit)*/

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT,500).getNotificationVal(),0xAUL);   /*  Notifier task send 0xA with FRTTransceiver::NotifyExtended 
                                                                                                                                                        We decrement it down to 0x9
                                                                                                                                                    */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT,500).getNotificationVal(),0x0UL);       /* 
                                                                                                                                                            The notifier sent 0x0, NofityReceiveBasic() 
                                                                                                                                                            wont accept this as a notification (only values >0 ) 
                                                                                                                                                            so we get the 0x0 but it will count as no notification
                                                                                                                                                        */
            assertEqual(commTest23.hasNotification(),false);

            assertEqual(commTest23.queueFlush(T_VALID,eMultiSenderQueue::eNOMULTIQSELECTED,true,0,true),true);          /* sync with notifier */

            assertEqual((unsigned long)commTest23.NotifyReceiveBasic(eFRTTNotifyActions::e_DECREMENTCOUNTONEXIT,500).getNotificationVal(),0x1UL);       /* Notifier does increment from 0x0 to 0x1, we decrement to 0x0 */
            assertEqual(commTest23.hasNotification(),true);

            assertEqual(commTest23.writeToQueue(T_VALID,0,&dummy,500,500,0),true);                                      /* sync with notifier */
            delayMicroseconds(300); /* Give notifier time to increment*/
            assertEqual((unsigned long)commTest23.NotifyReceiveExtended(0x0,0x1,500).getNotificationVal(),0x5UL);       /*  Notifier send 5x FRTT::FRTTransceiver::NotifyBasic(). 
                                                                                                                            We will receive 0x5 and do a clearOnExit with 0x1. clearOnExit(0x1,0x5) == 0x4 (b0100) */
        }

        int dummy = 15;
};


testF(NotificationTest,notificationtest)
{
    /* assertNoFatalFailure() stops this test if at least one assertXxx inside NotificationTest fails */

    assertNoFatalFailure(notifyWrongSetup_TEST1());

    assertNoFatalFailure(notifyReceiveBasic_TEST2());

    assertNoFatalFailure(notifyReceiveExtended_TEST3());

    assertNoFatalFailure(notifyReceiveMixed_TEST4());
}


void TestTask(void *)
{
    for(;;)
    {
        TestRunner::run();
    }
    vTaskDelete(nullptr);
}


void NotifierTask(void *)
{
    while(TEST_TASK == nullptr) vTaskDelay(pdMS_TO_TICKS(1));

    commNotifier.addDataAllocateCallback(dataAllocator);
    commNotifier.addDataFreeCallback(destroyer);

    commNotifier.addCommPartner(TEST_TASK,QSYNC_TEST2_AND3,3,SMPHR_TEST2_AND3,nullptr,0,nullptr,"SYNC-TO-TEST1");

    int dummy = 15;
    /* ----- Sequence start with TEST2 ---- */
    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_2*/

    commNotifier.NotifyBasic(TEST_TASK);                                                                /* incrementation notif.val of partner task ONCE*/
    
    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_2*/
    
    commNotifier.NotifyBasic(TEST_TASK);                                                                /* incrementation notif.val of partner task ONCE*/
    
    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_2*/
    
    commNotifier.NotifyBasic(TEST_TASK);                                                                /* incrementation notif.val of partner task 6 times*/
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);  

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_2*/

    commNotifier.NotifyBasic(TEST_TASK);                                                                /* incrementation notif.val of partner task 5 times*/
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    
    /* ----- Sequence finished with TEST2 ---- */
    /* ----- Sequence start with TEST3 ---- */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_NoAction,0);                            /* Set partner tasks notification state to pending */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetValueWithoutOverwrite,0xC);          /* Set partner tasks notification value to 0xC ----- 1100 */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_NoAction,0);                            /* Set partner tasks notification state to pending */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_NoAction,0);                            /* Set partner tasks notification state to pending */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetValueWithoutOverwrite,0xC);          /* Set partner tasks notification value to 0xC ----- 1100 */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_3 */

    delayMicroseconds(300);
    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_NoAction,0);                            /* Set partner tasks notification state to pending */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_Increment,0);

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetBits,0x1);                           /* Send 0x1 notif. value */
    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetValueWithOverwrite,0x2);             /* Overwrite 0x1 with 0x1 even though a notification was pending! */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_3 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetBits,0xF1F1F1F1);                    /* Send 0xF1F1F1F1 notif. value */

    /* ----- Sequence finished with TEST3 ---- */
    /* ----- Sequence started with TEST4 ---- */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_4 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetBits,0xA);                           /* Send 0xA with the extended version */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_4 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_SetValueWithoutOverwrite,0x0);          /* Send 0x0. the test_4 will not 'appect' this notification with FRTT::FRTTransceiver::NotifyReceiveBasic() */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) != 0);                                          /* synchronizing with test_4 */

    commNotifier.NotifyExtended(TEST_TASK,eFRTTNotifyActions::e_Increment,0x0);                         /* Increment. From 0x0 to 0x1. */

    while(commNotifier.messagesOnQueue(TEST_TASK,false) == 0);                                          /* synchronizing with test_4 */

    commNotifier.NotifyBasic(TEST_TASK);                                                                /* incrementation notif.val of partner task 5 times*/
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);                                                                
    commNotifier.NotifyBasic(TEST_TASK);   

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
      /* nothing to do */      
}


void setup()
{   
    disableCore0WDT();
    disableCore1WDT();
    vTaskDelay(pdMS_TO_TICKS(2500));
    Serial.begin(115200);               // For the internal printer of TestRunner
    while(!Serial);                     
    log_i("\n\n");


    log_i("This test should produce the following:");
    log_i("1 passed, 0 failed, 0 skipped, 0 timed out, out of 1 test(s).");
    log_i("----------\n\n");

    QSYNC_TEST2_AND3 = FRTTCreateQueue(3);
    
    SMPHR_TEST2_AND3 = FRTTCreateSemaphore();
    

    /* we dont need to aquire notifier-task's taskhandle since we ownly gonna 'READ' a notification, not send one..*/
    xTaskCreatePinnedToCore(TestTask,"test-task",5000,nullptr,8,&TEST_TASK,1);
    xTaskCreatePinnedToCore(NotifierTask,"notifier-task",5000,nullptr,8,nullptr,0);
}



void loop()
{
    //TestRunner::run();
} 