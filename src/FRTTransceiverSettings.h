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

/* used to select one of your multisender queues for a read operation. Add more if needed */
enum eMultiSenderQueue 
{
      eMULTISENDERQ0 = 0,
      eMULTISENDERQ1 = 1,
      eMULTISENDERQ2 = 2,
      eMULTISENDERQ3 = 3,
      eMULTISENDERQ4 = 4
};

#endif