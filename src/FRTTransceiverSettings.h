#ifndef FRTTSETTINGS_H
#define FRTTSETTINGS_H

/*!
 * \file       FRTTransceiverSettings.h
 * \brief      Library configurations
 * \author     Xhemail Ramabaja (x.ramabaja@outlook.de)
 *
 */


//#define FRTTRANSCEIVER_64BITADDITIONALDATA
#define FRTTRANSCEIVER_32BITADDITIONALDATA

/**
 * Keep it as small as possible to save memory (only sizes > 0 valid) 
 * 
 */ 
#define FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE          (4u)

#define FRTTRANSCEIVER_DEFAULTPARTNERNAME             ("UNKNOWN NAME") 
#define FRTTRANSCEIVER_MULTISENDERDEFAULTPARTNERNAME  ("MULTISENDER-QUEUE")

#define FRTTRANSCEIVER_ANALYTICS_ENABLE

#if defined(FRTTRANSCEIVER_ANALYTICS_ENABLE)

#define FRTTRANSCEIVER_COMMTYPE1                      ("NORMAL COMMUNICATION (READ & WRITE)")
#define FRTTRANSCEIVER_COMMTYPE2                      ("MULTISENDER QUEUE (READONLY)")
#define FRTTRANSCEIVER_UNKNOWNADDRESS                 (0x00000000)
#define FRTTRANSCEIVER_UNKNOWNNAME                    ("UKNOWN NAME")
#endif

/**
 * Normally one would need to select a communication line by supplying a FRTTransceiver_TaskHandle.
 * A multisender queue does not have a unique FRTTransceiver_TaskHandle, so that another way of selecting a line had to be introduced.
 * 
 * enum eMultiSenderQueue is used to select one of your multisender queues for a read operation. 
 * 
 * Meaning: First multi-sender queue can be selected by using eMULTISENDERQ0
 *          Second multi-sender queue can be selected by using eMULTISENDERQ1 ... and so on.
 * 
 * There should be a maximum of FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE enum keys. 
 *          ---> Increasing FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE also requires one to adjust the amount of eMultiSenderQueue keys to match FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE
 * 
 */

enum eMultiSenderQueue 
{
      eNOMULTIQSELECTED   = -1,
      eMULTISENDERQ0 = 1,
      eMULTISENDERQ1 = 2,
      eMULTISENDERQ2 = 3,
      eMULTISENDERQ3 = 4,
      eMULTISENDERQ4 = 5,
      eMULTISENDERQ5 = 6,
      eMULTISENDERQ6 = 7,
      eMULTISENDERQ7 = 8,
      eMULTISENDERQ8 = 9
};

#endif