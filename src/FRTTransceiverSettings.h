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

// Adjust as needed. Keep it as small as possible to save memory (only sizes > 0 valid)
#define FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE (4u)

#define FRTTRANSCEIVER_MULTISENDERDEFAULTPARTNERNAME ("MULTISENDER-QUEUE")

/* add more if needed */
enum eMultiSenderQueue 
{
      eQUEUE1 = 1,
      eQUEUE2 = 2,
      eQUEUE3 = 3,
      eQUEUE4 = 5,
      eQUEUE5 = 6
};

#endif