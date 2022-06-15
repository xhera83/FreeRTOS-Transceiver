
#include <AUnit.h>
#include "BufferTests.h"

using namespace aunit;

#if (DYNAMIC_STRUCTARRAY == 1)
FRTTransceiver comm1(T_VALID,1);
FRTTransceiver comm2(T_VALID,1);
FRTTransceiver comm3(T_VALID,1);
FRTTransceiver comm4(T_VALID,1);               
FRTTransceiver comm5(T_VALID,2);                
FRTTransceiver comm6(T_VALID,1);                
#else
FRTTCommunicationPartner partner1[1];
FRTTCommunicationPartner partner2[1];
FRTTCommunicationPartner partner3[1];
FRTTCommunicationPartner partner4[1];
FRTTCommunicationPartner partner5[2];
FRTTCommunicationPartner partner6[1];

FRTTransceiver comm1(T_VALID,&partner1[0],1);
FRTTransceiver comm2(T_VALID,&partner2[0],1);
FRTTransceiver comm3(T_VALID,&partner3[0],1);
FRTTransceiver comm4(T_VALID,&partner4[0],1);
FRTTransceiver comm5(T_VALID,&partner5[0],2);
FRTTransceiver comm6(T_VALID,&partner6[0],1);
#endif


test(manualDeleteBufferPositions_TEST1)
{
    /* Manual deletion of buffer position.*/

    /* Is an echo communication, same tests can be made with normal communication/multi-sender-queue  */
    comm1.addDataAllocateCallback(dataAllocator);
    comm1.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);

    int writeOperation1 = 1;
    int writeOperation2 = 2;
    int writeOperation3 = 3;

    assertEqual(comm1.addCommPartner(TASK1,QUEUE,3,SMPHR,QUEUE,3,SMPHR,("ECHO")),true);

    assertEqual(comm1.writeToQueue(TASK1,0,&writeOperation1,100,100,0),true);

    assertEqual(comm1.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);

    assertEqual(comm1.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),1);
    assertEqual(comm1.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),true);
    assertEqual(comm1.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),0);

    assertEqual(comm1.writeToQueue(TASK1,0,&writeOperation1,100,100,0),true);
    assertEqual(comm1.writeToQueue(TASK1,0,&writeOperation2,100,100,0),true);
    assertEqual(comm1.writeToQueue(TASK1,0,&writeOperation3,100,100,0),true);

    assertEqual(comm1.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);
    assertEqual(comm1.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);
    assertEqual(comm1.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);

    assertEqual(comm1.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,3),false);   /* Index too big (0,1,2 possible) */
    assertEqual(comm1.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,2),true);    /* newest data */
    assertEqual(comm1.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),true);    /* oldest */
    assertEqual(comm1.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),true);    /* remaining data*/

    assertEqual(comm1.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),0);

    assertEqual(comm1.delDatabuffForLine((FRTTTaskHandle)(T_VALID + 0xA),eMultiSenderQueue::eNOMULTIQSELECTED,true,0),false);  /* Unknown address */
    comm1.~FRTTransceiver();
}

test(manualDeleteAllData_TEST2)
{
    /*  Manual deletion of all buffer position at once */


    /* Is an echo communication, same tests can be made with normal communication/multi-sender-queue  */
    comm2.addDataAllocateCallback(dataAllocator);
    comm2.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);

    int writeOperation1 = 1;
    int writeOperation2 = 2;
    int writeOperation3 = 3;

    assertEqual(comm2.addCommPartner(TASK1,QUEUE2,3,SMPHR2,QUEUE2,3,SMPHR2,("ECHO")),true);

    assertEqual(comm2.writeToQueue(TASK1,0,&writeOperation1,100,100,0),true);
    assertEqual(comm2.writeToQueue(TASK1,0,&writeOperation2,100,100,0),true);
    assertEqual(comm2.writeToQueue(TASK1,0,&writeOperation3,100,100,0),true);

    assertEqual(comm2.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);
    assertEqual(comm2.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);
    assertEqual(comm2.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);

    assertEqual(comm2.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),3);

    assertEqual(comm2.delAllDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),true);
    assertEqual(comm2.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),0);
    assertEqual(comm2.hasDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),false);

    assertEqual(comm2.delAllDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),false); /* Nothing to flush */
    assertEqual(comm2.delAllDatabuffForLine((FRTTTaskHandle)(T_VALID + 0xA),eMultiSenderQueue::eNOMULTIQSELECTED,true),false); /* Unknown address  */
    comm2.~FRTTransceiver();
}

test(bufferRearrangement_TEST3)
{
    /* Manual deletion of buffer position. Checks if buffer rearranges it self in the right order */
    /* Is an echo communication, same tests can be made with normal communication/multi-sender-queue  */
    comm3.addDataAllocateCallback(dataAllocator);
    comm3.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);

    int writeOperation1 = 0;
    int writeOperation2 = 2;
    int writeOperation3 = 4;

    assertEqual(comm3.addCommPartner(TASK1,QUEUE3,3,SMPHR3,QUEUE3,3,SMPHR3,("ECHO")),true);
    assertEqual(comm3.writeToQueue(TASK1,0,&writeOperation1,100,100,0),true);                                           /* POS 0 - writeOperation1 variable */
    assertEqual(comm3.writeToQueue(TASK1,0,&writeOperation2,100,100,0),true);                                           /* POS 1 - writeOperation2 variable*/
    assertEqual(comm3.writeToQueue(TASK1,0,&writeOperation3,100,100,0),true);                                           /* POS 2 - writeOperation3 variable*/

    assertEqual(comm3.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                     /* MOVED INTO BUFFER */
    assertEqual(comm3.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                     /* MOVED INTO BUFFER */
    assertEqual(comm3.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                     /* MOVED INTO BUFFER */

    assertEqual(comm3.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,1),true);                      /* POS 1 REMOVED (writeOperation2 variable)*/
    assertEqual(*((int *)comm3.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,1)->data),4);        /* SHOULD BE writeOperation3 value */

    // POSITIONS LEFT: POS0 (writeOperation1), POS1 (writeOperation3)

    assertEqual(comm3.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),true);                      /* POS 0 REMOVED (writeOperation1 variable)*/
    assertEqual(*((int *)comm3.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0)->data),4);        /* SHOULD BE writeOperation3 value */

    // POSITIONS LEFT: POS0 (writeOperation3)

    assertEqual(comm3.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),true);                      /* POS 0 REMOVED (writeOperation3 variable)*/
    assertEqual(comm3.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),false);                     /* NO BUFFER LEFT, SO FALSE */
    assertEqual(comm3.delDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,1),false);                     /* NO BUFFER LEFT, SO FALSE */  
    comm3.~FRTTransceiver();
}

test(bufferRearrangement2_TEST4)
{
    /* Manual deletion of buffer position. Checks if buffer rearranges it self in the right way. 
       Reading more data from the queue than the buffers can hold will make the library automatically delete the oldest data and rearrange the buffer new (shift positions )
    */

    /* Is an echo communication, same tests can be made with normal communication/multi-sender-queue  */
    comm4.addDataAllocateCallback(dataAllocator);
    comm4.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);

    int writeOperation1 = 0;
    int writeOperation2 = 2;
    int writeOperation3 = 4;
    int writeOperation4 = 6;
    int writeOperation5 = 8;
    int writeOperation6 = 10;

    assertEqual(comm4.addCommPartner(TASK1,QUEUE4,3,SMPHR4,QUEUE4,3,SMPHR4,("ECHO")),true);

    assertEqual(comm4.writeToQueue(TASK1,0,&writeOperation1,100,100,0),true);                                               /* POS 0 - writeOperation1 variable */
    assertEqual(comm4.writeToQueue(TASK1,0,&writeOperation2,100,100,0),true);                                               /* POS 1 - writeOperation2 variable */
    assertEqual(comm4.writeToQueue(TASK1,0,&writeOperation3,100,100,0),true);                                               /* POS 2 - writeOperation3 variable */

    assertEqual(comm4.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                         /* MOVED INTO BUFFER */
    assertEqual(comm4.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                         /* MOVED INTO BUFFER */
    assertEqual(comm4.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                         /* MOVED INTO BUFFER */

    // BUFFER: POS0 (writeOperation1), POS1 (writeOperation2), POS2 (writeOperation3) ---> BUFFER FULL, POS0 IS THE OLDEST DATA

    assertEqual(comm4.writeToQueue(TASK1,0,&writeOperation4,100,100,0),true);                                               /* POS 0 - writeOperation4 variable */
    assertEqual(comm4.writeToQueue(TASK1,0,&writeOperation5,100,100,0),true);                                               /* POS 1 - writeOperation5 variable */
    assertEqual(comm4.writeToQueue(TASK1,0,&writeOperation6,100,100,0),true);                                               /* POS 2 - writeOperation6 variable */
    
    assertEqual(comm4.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                         /* MOVED INTO BUFFER 
                                                                                                                                            POS0 REMOVED, NEW DATA ON POS2*/
    
    // BUFFER: POS0 (writeOperation2), POS1 (writeOperation3), POS2 (writeOperation4) ---> BUFFER FULL, POS0 IS THE OLDEST DATA

    assertEqual(*((int *)comm4.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0)->data),2);            /* SHOULD BE writeOperation2 value */ 

    assertEqual(comm4.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                         /* MOVED INTO BUFFER 
                                                                                                                                             POS0 REMOVED, NEW DATA ON POS2*/

    // BUFFER: POS0 (writeOperation3), POS1 (writeOperation4), POS2 (writeOperation5) ---> BUFFER FULL, POS0 IS THE OLDEST DATA

    assertEqual(*((int *)comm4.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0)->data),4);            /* SHOULD BE writeOperation3 value */

    assertEqual(comm4.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                         /* MOVED INTO BUFFER 
                                                                                                                                             POS0 REMOVED, NEW DATA ON POS2*/

    // BUFFER: POS0 (writeOperation4), POS1 (writeOperation5), POS2 (writeOperation6) ---> BUFFER FULL, POS0 IS THE OLDEST DATA

    assertEqual(*((int *)comm4.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0)->data),6);            /* SHOULD BE writeOperation4 value */
    assertEqual(*((int *)comm4.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,1)->data),8);            /* SHOULD BE writeOperation5 value */
    assertEqual(*((int *)comm4.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,2)->data),10);           /* SHOULD BE writeOperation6 value */
    comm4.~FRTTransceiver();
}


test(bufferTools_TEST5)
{
    /*  Checks if some important methods work as expected */
    /*  Is an echo communication, same tests can be made with normal communication/multi-sender-queue  */

    comm5.addDataAllocateCallback(dataAllocator);
    comm5.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID + 0x1);
    FRTTTaskHandle TASK2 = (FRTTTaskHandle)(T_VALID + 0x2);

    int writeOperation1 = 0;
    std::string writeOperation2("writeOperation2");
    char * writeOperation3 = "writeOperation3";
    int writeOperation4 = 6;
    int writeOperation5 = 8;
    int writeOperation6 = 10;

    assertEqual(comm5.addCommPartner(TASK1,QUEUE5,3,SMPHR5,QUEUE5,3,SMPHR5,("ECHO")),true);
    assertEqual(comm5.addCommPartner(TASK2,QUEUE6,3,SMPHR6,QUEUE6,3,SMPHR6,("ECHO2")),true);

    assertEqual(comm5.writeToQueue(TASK1,(uint8_t)Datatypes::eINTPTR,&writeOperation1,100,100,0),true);
    assertEqual(comm5.writeToQueue(TASK1,(uint8_t)Datatypes::eSTRINGPTR,&writeOperation2,100,100,0),true);                          
    assertEqual(comm5.writeToQueue(TASK1,(uint8_t)Datatypes::eCHARPTR,&writeOperation3[0],100,100,0),true);

    assertEqual(comm5.writeToQueue(TASK2,(uint8_t)Datatypes::eINTPTR,&writeOperation1,100,100,0),true);
    assertEqual(comm5.writeToQueue(TASK2,(uint8_t)Datatypes::eSTRINGPTR,&writeOperation2,100,100,0),true);
    assertEqual(comm5.writeToQueue(TASK2,(uint8_t)Datatypes::eINTPTR,&writeOperation1,100,100,0),true); 

    assertEqual(comm5.bufferedDataInAllBuffers(),0);
    assertEqual(comm5.hasDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),false);                                      /* NO DATA IN BUFFER */
    assertEqual(comm5.hasDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),false);                                      /* NO DATA IN BUFFER */ 

    assertEqual(comm5.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                             /* MOVED INTO BUFFER COMM1 */

    assertEqual(comm5.bufferedDataInAllBuffers(),1);                                                                            /* DATA IN ALL BUFFER: 1 */
    assertEqual(comm5.hasDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),true);                                       /* DATA IN BUFFER */
    assertEqual(comm5.hasDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),false);                                      /* NO DATA IN BUFFER */ 
    assertEqual(comm5.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),1);                                     /* SHOULD HAVE 1 MESSAGE IN BUFFER*/
    assertEqual(comm5.bufferedDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),0);                                     /* SHOULD HAVE 0 MESSAGES IN BUFFER*/

    assertEqual(comm5.readFromQueue(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                             /* MOVED INTO BUFFER COMM2*/                        

    assertEqual(comm5.bufferedDataInAllBuffers(),2);                                                                            /* DATA IN ALL BUFFER: 2 */
    assertEqual(comm5.hasDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),true);                                       /* NOW WE HAVE DATA IN BUFFER */
    assertEqual(comm5.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),1);                                     /* SHOULD STILL HAVE 1 MESSAGE IN BUFFER*/
    assertEqual(comm5.bufferedDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),1);                                     /* SHOULD HAVE 1 MESSAGE IN BUFFER*/

    assertEqual(comm5.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                             /* MOVED INTO BUFFER COMM1*/
    assertEqual(comm5.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                             /* MOVED INTO BUFFER COMM1*/
    assertEqual(comm5.readFromQueue(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                             /* MOVED INTO BUFFER COMM2*/
    assertEqual(comm5.readFromQueue(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                             /* MOVED INTO BUFFER COMM2*/

    assertEqual(comm5.bufferedDataInAllBuffers(),6);                                                                            /* DATA IN ALL BUFFER: 6 */

    /* Datatypes in buffer for COMM1 -> POS0 (eINTPTR), POS1 (eSTRINGPTR), POS2 (eCHARPTR)*/
    assertEqual(comm5.isDatatypeInBuffer(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eINTPTR),1);       /* One pointer to int expected */
    assertEqual(comm5.isDatatypeInBuffer(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eSTRINGPTR),1);    /* One pointer to std::string expected */
    assertEqual(comm5.isDatatypeInBuffer(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eCHARPTR),1);      /* One pointer to c-string expected */

    /* Datatypes in buffer for COMM2 -> POS0 (eINTPTR), POS1 (eSTRINGPTR), POS2 (eINTPTR)*/
    assertEqual(comm5.isDatatypeInBuffer(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eINTPTR),2);       /* TWO pointer to int expected */
    assertEqual(comm5.isDatatypeInBuffer(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eSTRINGPTR),1);    /* One pointer to std::string expected */
    assertEqual(comm5.isDatatypeInBuffer(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eCHARPTR),0);      /* ZERO pointer to c-string expected */

    assertEqual(comm5.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),3);                                     /* SHOULD HAVE 3 MESSAGE IN BUFFER*/
    assertEqual(comm5.bufferedDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),3);                                     /* SHOULD HAVE 3 MESSAGE IN BUFFER*/

    assertEqual(comm5.delAllDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),true);                             /* FLUSH BUFFER COMM1*/

    /* Datatypes in buffer for COMM1 -> POS0 (EMPTY), POS1 (EMPTY), POS2 (EMPTY)*/
    assertEqual(comm5.isDatatypeInBuffer(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eINTPTR),0);       /* ZERO pointer to int expected */
    assertEqual(comm5.isDatatypeInBuffer(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eSTRINGPTR),0);    /* ZERO pointer to std::string expected */
    assertEqual(comm5.isDatatypeInBuffer(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,(uint8_t)Datatypes::eCHARPTR),0);      /* ZERO pointer to c-string expected */

    assertEqual(comm5.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),0);                                     /* SHOULD HAVE 0 MESSAGE IN BUFFER*/
    assertEqual(comm5.hasDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),false);                                      /* DATE REMOVED -> NO DATA IN  BUFFER */
    assertEqual(comm5.bufferedDataInAllBuffers(),3);                                                                            /* DATA IN ALL BUFFER: 3 */
    assertEqual(comm5.delAllDatabuffForLine(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),true);                             /* FLUSH BUFFER COMM2*/

    assertEqual(comm5.bufferedDataInAllBuffers(),0);                                                                            /* DATA IN ALL BUFFER: 0 */
    assertEqual(comm5.bufferedDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),0);                                     /* SHOULD HAVE 0 MESSAGE IN BUFFER*/
    assertEqual(comm5.hasDataFrom(TASK2,eMultiSenderQueue::eNOMULTIQSELECTED,true),false);                                      /* DATE REMOVED -> NO DATA IN  BUFFER */
    comm5.~FRTTransceiver();
    
}

test(bufferData_TEST6)
{
    /*  Checks if the library passes/reads data in the right order */
    /*  Is an echo communication, same tests can be made with normal communication/multi-sender-queue  */
    comm6.addDataAllocateCallback(dataAllocator);
    comm6.addDataFreeCallback(destroyer);

    FRTTTaskHandle TASK1 = (FRTTTaskHandle)(T_VALID);

    int a = 15;
    std::string cppString("This is a cpp string.");
    char * charptr = "This is a c string";


    assertEqual(comm6.addCommPartner(TASK1,QUEUE7,3,SMPHR7,QUEUE7,3,SMPHR7,"ECHO2"),true);

    assertEqual(comm6.writeToQueue(TASK1,(uint8_t)Datatypes::eCHARPTR,&charptr[0],100,100,500),true);                
    assertEqual(comm6.writeToQueue(TASK1,(uint8_t)Datatypes::eSTRINGPTR,&cppString,100,100,1000),true);                                                                                     
    assertEqual(comm6.writeToQueue(TASK1,(uint8_t)Datatypes::eINTPTR,&a,100,100,1500),true);  

    assertEqual(comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),nullptr);                    /* NO DATA IN BUFFER POS 0*/
    assertEqual(comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),nullptr);                /* SAME AS ABOVE */
    assertEqual(comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),nullptr);                /* SAME AS ABOVE */

    assertEqual(comm6.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                    /* MOVE INTO BUFFER */
    assertEqual(comm6.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                    /* MOVE INTO BUFFER */   
    assertEqual(comm6.readFromQueue(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,500,500),true);                    /* MOVE INTO BUFFER */

    assertEqual(comm6.bufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),3);

    assertNotEqual(comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),nullptr);                    /* WILL NOT BE NULL */
    assertNotEqual(comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,1),nullptr);                    /* WILL NOT BE NULL */
    assertNotEqual(comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,2),nullptr);                    /* WILL NOT BE NULL */
    assertNotEqual(comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),nullptr);                /* WILL NOT BE NULL */
    assertNotEqual(comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),nullptr);                /* WILL NOT BE NULL */

    /* Datatypes in buffer -> POS0 (eCHARPTR)(add.Data = 500), POS1 (eSTRINGPTR)(add.Data = 1000), POS2 (eINTPTR)(add.Data = 1500)*/

    const FRTTTempDataContainer * t1 = comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0);    /* OLDEST DATA */
    const FRTTTempDataContainer * t2 = comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,1);
    const FRTTTempDataContainer * t3 = comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,2);    /* NEWEST DATA */
    
    assertEqual(t1->senderAddress,TASK1);                       
    assertEqual(t1->u32AdditionalData,(uint32_t)500);
    assertEqual(t1->u8DataType,(uint8_t)Datatypes::eCHARPTR);
    assertEqual((char*)t1->data,(char *)"This is a c string");

    assertEqual(t2->senderAddress,TASK1);
    assertEqual(t2->u32AdditionalData,(uint32_t)1000);
    assertEqual(t2->u8DataType,(uint8_t)Datatypes::eSTRINGPTR);
    assertEqual(((std::string *)t2->data)->c_str(),"This is a cpp string.");

    assertEqual(t3->senderAddress,TASK1);
    assertEqual(t3->u32AdditionalData,(uint32_t)1500);
    assertEqual(t3->u8DataType,(uint8_t)Datatypes::eINTPTR);
    assertEqual(*((int *)t3->data),15);

    /* Same tests as with t1 and t3 pointer but with getOldestBufferedData/getNewestBufferedData methods */

    assertEqual(((const FRTTTempDataContainer *)comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->senderAddress,TASK1);
    assertEqual(((const FRTTTempDataContainer *)comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->u32AdditionalData,(uint32_t)500);
    assertEqual(((const FRTTTempDataContainer *)comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->u8DataType,(uint8_t)Datatypes::eCHARPTR);
    assertEqual((char*)((const FRTTTempDataContainer *)comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->data,(char *)"This is a c string");

    assertEqual(((const FRTTTempDataContainer *)comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->senderAddress,TASK1);
    assertEqual(((const FRTTTempDataContainer *)comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->u32AdditionalData,(uint32_t)1500);
    assertEqual(((const FRTTTempDataContainer *)comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->u8DataType,(uint8_t)Datatypes::eINTPTR);
    assertEqual(*((int *)((const FRTTTempDataContainer *)comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true))->data),15);

    assertEqual(comm6.delAllDatabuffForLine(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),true);   /* FLUSH BUFFER */
    assertEqual(comm6.getBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true,0),nullptr);                        /* NO DATA IN BUFFER POS 0*/
    assertEqual(comm6.getNewestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),nullptr);                    /* SAME AS ABOVE */
    assertEqual(comm6.getOldestBufferedDataFrom(TASK1,eMultiSenderQueue::eNOMULTIQSELECTED,true),nullptr);                    /* SAME AS ABOVE */
}

void setup()
{
    vTaskDelay(pdMS_TO_TICKS(2500));
    Serial.begin(115200);               // For the internal printer of TestRunner
    while(!Serial);                     
    log_i("\n\n");


    QUEUE = FRTTCreateQueue(3);
    QUEUE2 = FRTTCreateQueue(3);
    QUEUE3 = FRTTCreateQueue(3);
    QUEUE4 = FRTTCreateQueue(3);

    QUEUE5 = FRTTCreateQueue(3);
    QUEUE6 = FRTTCreateQueue(3);
    QUEUE7 = FRTTCreateQueue(3);

    SMPHR = FRTTCreateSemaphore();
    SMPHR2 = FRTTCreateSemaphore();
    SMPHR3 = FRTTCreateSemaphore();
    SMPHR4 = FRTTCreateSemaphore();
    
    SMPHR5 = FRTTCreateSemaphore();
    SMPHR6 = FRTTCreateSemaphore();
    SMPHR7 = FRTTCreateSemaphore();

    log_i("This test should produce the following:");
    log_i("6 passed, 0 failed, 0 skipped, 0 timed out, out of 6 test(s).");
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