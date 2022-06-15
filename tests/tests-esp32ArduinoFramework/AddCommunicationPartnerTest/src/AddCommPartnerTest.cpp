
#include <AUnit.h>
#include "FRTTransceiver.h"
#include "AddCommPartnerTest.h"

using namespace aunit;

/* No need to check the dynamic version since the library handles everything on its own */
#if (DYNAMIC_STRUCTARRAY == 0)
test(constructor)
{
    
        FRTTCommunicationPartner partners5[1];

        FRTTransceiver comm4(T_VALID,nullptr,15);          /* In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously  */
        FRTTransceiver comm5(T_VALID,&partners5[0],0);     /* In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously  */

        assertEqual(comm4.addCommPartner(T_VALID,Q_VALID,1,S_VALID,nullptr,0,nullptr,PARTNERNAME),false);   /* we didnt provide a valid communication struct (see ~2 lines above)           */
        assertEqual(comm5.addCommPartner(T_VALID,Q_VALID,1,S_VALID,nullptr,0,nullptr,PARTNERNAME),false);   /* we didnt provide a valid struct length (see ~2 lines above)                  */
    
}
#endif


test(allPossibleParams)
{   
    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(T_VALID,5); /* In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously */
    #else
    FRTTCommunicationPartner partners[5];
    FRTTransceiver comm(T_VALID,&partners[0],5);    /* In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously */
    #endif

    /* Partner tasks must be distinguishable */
    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,1,S_VALID,Q_VALID2,1,S_VALID2,PARTNERNAME),true);                       /* 4 Left   */     
    assertEqual(comm.addCommPartner(T_VALID2,Q_INVALID,0,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),true);          /* 3 LEFT   */ 

    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID,1,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);           /* RX Queue exists               (rx)    */
    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID2,1,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);          /* RX Queue exists               (tx)    */  
    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID3,1,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);          /* SEM-RX nullptr                        */
    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID3,1,S_VALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);            /* SEM-exists butt already taken (rx)    */
    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID3,1,S_VALID2,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);           /* SEM-exists butt already taken (tx)    */
    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID3,700,S_VALID3,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);         /* wrong queue length                    */

    assertEqual(comm.addCommPartner(T_VALID3,Q_INVALID,0,S_INVALID,Q_VALID,0,S_INVALID,PARTNERNAME_EMPTY),false);           /* TX Queue exists               (rx)    */
    assertEqual(comm.addCommPartner(T_VALID3,Q_INVALID,0,S_INVALID,Q_VALID2,0,S_INVALID,PARTNERNAME_EMPTY),false);          /* TX Queue exists               (tx)    */  
    assertEqual(comm.addCommPartner(T_VALID3,Q_INVALID,0,S_INVALID,Q_VALID3,0,S_INVALID,PARTNERNAME_EMPTY),false);          /* SEM-RX nullptr                        */
    assertEqual(comm.addCommPartner(T_VALID3,Q_INVALID,0,S_INVALID,Q_VALID3,0,S_VALID,PARTNERNAME_EMPTY),false);            /* SEM-exists butt already taken (rx)    */
    assertEqual(comm.addCommPartner(T_VALID3,Q_INVALID,0,S_INVALID,Q_VALID3,0,S_VALID2,PARTNERNAME_EMPTY),false);           /* SEM-exists butt already taken (tx)    */
    assertEqual(comm.addCommPartner(T_VALID3,Q_INVALID,0,S_INVALID,Q_VALID3,152,S_VALID3,PARTNERNAME_EMPTY),false);         /* wrong queue length                    */

    assertEqual(comm.addCommPartner(T_VALID3,Q_VALID3,1,S_VALID3,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),true);        /* 2 LEFT   */
    assertEqual(comm.addCommPartner(T_VALID4,Q_INVALID,0,S_INVALID,Q_VALID4,2,S_VALID4,PARTNERNAME_EMPTY),true);        /* 1 LEFT   */
    assertEqual(comm.addCommPartner(T_VALID5,Q_VALID5,1,S_VALID5,Q_VALID6,2,S_VALID6,PARTNERNAME_EMPTY),true);          /* 0 LEFT   */
    assertEqual(comm.addCommPartner(T_VALID6,Q_INVALID,0,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);     /* Should block right away: no more connections   */
    assertEqual(comm.addCommPartner(T_VALID6,Q_INVALID,0,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);     /* Should block right away: no more connections   */
    assertEqual(comm.addCommPartner(T_VALID6,Q_INVALID,0,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);     /* Should block right away: no more connections   */
    assertEqual(comm.addCommPartner(T_VALID6,Q_INVALID,0,S_INVALID,Q_INVALID,0,S_INVALID,PARTNERNAME_EMPTY),false);     /* Should block right away: no more connections   */

    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm2(T_VALID,6); /* In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously */
    #else
    FRTTCommunicationPartner partners2[6];
    FRTTransceiver comm2(T_VALID,&partners2[0],6);    /* In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously */
    #endif


    
    assertEqual(comm2.addCommPartner(T_VALID,Q_INVALID,0,S_INVALID,Q_VALID3,1,S_VALID3,PARTNERNAME),true);  /* 5 left, to have tx queue & semph */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID,1,S_VALID,PARTNERNAME),true);                           /* 4 left */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID2,1,S_VALID2,PARTNERNAME),true);                         /* 3 left */

    assertEqual(comm2.addMultiSenderPartner(Q_INVALID,0,S_INVALID,PARTNERNAME),false);                      /* Queue nullptr        */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID,0,S_INVALID,PARTNERNAME),false);                        /* Queue exists rx      */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID3,0,S_INVALID,PARTNERNAME),false);                       /* Queue exists tx      */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID4,0,S_INVALID,PARTNERNAME),false);                       /* SMPH nullptr         */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID4,0,S_VALID3,PARTNERNAME),false);                        /* SMPH exists tx       */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID4,0,S_VALID2,PARTNERNAME),false);                        /* SMPH exists rx       */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID4,124,S_VALID4,PARTNERNAME),false);                      /* wrong queue length   */

    assertEqual(comm2.addMultiSenderPartner(Q_VALID4,1,S_VALID4,PARTNERNAME),true);                         /* 2 left */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID5,1,S_VALID5,PARTNERNAME),true);                         /* 1 left */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID6,1,S_VALID6),true);                                     /* 0 left */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID6,1,S_VALID6),false);                                    /* 0 left */
    assertEqual(comm2.addMultiSenderPartner(Q_VALID6,1,S_VALID6),false);                                    /* 0 left */
}

                              
test(combinedAdditionToCommunicationList)
{
    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(T_VALID,4); // In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously 
    #else
    FRTTCommunicationPartner partners[4];
    FRTTransceiver comm(T_VALID,&partners[0],4);    // In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously 
    #endif

    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,1,S_VALID,Q_VALID,1,S_VALID,PARTNERNAME),true);
    assertEqual(comm.addMultiSenderPartner(Q_VALID2,1,S_VALID2,PARTNERNAME),true);

    assertEqual(comm.addMultiSenderPartner(Q_INVALID,1,S_INVALID,PARTNERNAME),false); 
    assertEqual(comm.addCommPartner(T_INVALID,Q_INVALID,1,S_INVALID,Q_INVALID,1,S_INVALID,PARTNERNAME),false);

    assertEqual(comm.addMultiSenderPartner(Q_VALID3,1,S_VALID3,PARTNERNAME),true);
    assertEqual(comm.addCommPartner(T_VALID2,Q_VALID4,1,S_VALID4,Q_VALID5,1,S_VALID5,PARTNERNAME),true);

    assertEqual(comm.addMultiSenderPartner(Q_VALID,1,S_VALID,PARTNERNAME),false);
    assertEqual(comm.addCommPartner(T_VALID + 0x2,Q_VALID,1,S_VALID,Q_VALID,1,S_VALID,PARTNERNAME),false);
}

                                   
test(queueLengthTest)
{
    /* Only works with FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE == 3*/

    #if (DYNAMIC_STRUCTARRAY == 1)
    FRTTransceiver comm(T_VALID,2); // In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously 
    #else
    FRTTCommunicationPartner partners[2];
    FRTTransceiver comm(T_VALID,&partners[0],2);    // In this current version of the library it doesnt matter if we add a valid owners address or not. A task can work anonymously 
    #endif

    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,4,S_VALID,Q_VALID,1,S_VALID,PARTNERNAME),false);
    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,1,S_VALID,Q_VALID,4,S_VALID,PARTNERNAME),false);
    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,0,S_VALID,Q_VALID,1,S_VALID,PARTNERNAME),false);
    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,1,S_VALID,Q_VALID,0,S_VALID,PARTNERNAME),false);

    assertEqual(comm.addCommPartner(T_VALID,Q_VALID,FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE,S_VALID,Q_VALID2,1,S_VALID2,PARTNERNAME),true);        /* 1 LEFT */

    assertEqual(comm.addMultiSenderPartner(Q_VALID3,FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE + 1,S_VALID3,PARTNERNAME),false);
    assertEqual(comm.addMultiSenderPartner(Q_VALID3,0,S_VALID3,PARTNERNAME),false);

    assertEqual(comm.addMultiSenderPartner(Q_VALID3,2,S_VALID3,PARTNERNAME),true);                                                               /* 0 Left */


    assertEqual(comm.addCommPartner(T_VALID2,Q_VALID4,FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE,S_VALID4,Q_VALID5,1,S_VALID5,PARTNERNAME),false);     /* 0 Left...*/
}


void setup()
{
    vTaskDelay(pdMS_TO_TICKS(2500));
    Serial.begin(115200);               // For the internal printer of TestRunner
    while(!Serial);                     
    log_i("\n\n");
    
    log_i("This test should produce the following:");
    #if (DYNAMIC_STRUCTARRAY == 0)
    log_i("4 passed, 0 failed, 0 skipped, 0 timed out, out of 4 test(s).");
    #elif (DYNAMIC_STRUCTARRAY == 1 )
    log_i("3 passed, 0 failed, 0 skipped, 0 timed out, out of 3 test(s).");
    #endif
    log_i("----------\n\n");
}


void loop()
{
    TestRunner::run();
}