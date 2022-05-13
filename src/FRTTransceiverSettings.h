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
#define FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE (3u)

#endif