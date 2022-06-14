#ifndef FRTTRANSCEIVERSETTINGS_H
#define FRTTRANSCEIVERSETTINGS_H

/*!
 * \file        FRTTransceiverSettings.h
 * \brief       Configuration file for the FreeRTOS-Transceiver library
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version     v1.1.0
 * \copyright   Copyright 2022 Xhemail Ramabaja
 *
 */

#include <stdint.h>

//*! \brief TURN ON (uncomment) to use up to 64 bit as additional data for a data package on queue (turn off 32 bit)*/
//#define FRTTRANSCEIVER_64BITADDITIONALDATA
/*! \brief TURN ON (uncomment) to use up to 32 bit as additional data for a data package on queue (turn off 64 bit)*/
#define FRTTRANSCEIVER_32BITADDITIONALDATA

/*!
 * \brief   Defines the maximum length a queue can have
 * \note    To save memory please set the number as low as possible (> 0)
 */
#define FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE                (3u)

/*! \brief   Default name of a partner, when no name is supplied */
#define FRTTRANSCEIVER_DEFAULTPARTNERNAME                   ("UNKNOWN NAME") 
/*! \brief Default name of a multi-sender-queue (multiple tasks write on the tx line), when no name is supplied */
#define FRTTRANSCEIVER_DEFAULTPARTNERNAMEMULTISENDERQUEUE   ("MULTISENDER-QUEUE")
/*! \brief TURN ON (uncomment) if you want to use FRTTransceiver::printCommunicationsSummary() and let the library track packages received/sent */
#define FRTTRANSCEIVER_ANALYTICS_ENABLE

#if defined(FRTTRANSCEIVER_ANALYTICS_ENABLE)
/*!
 * \brief   Describes a normal communication line with another task
 * \note    Used for FRTTransceiver#printCommunicationsSummary
 */
#define FRTTRANSCEIVER_COMMTYPE1                            ("NORMAL COMMUNICATION (READ & WRITE)")
/*!
 * \brief   Describes a read-only communication line, where the tx queue is a multi-sender-queue (multiple tasks write on the tx line)
 * \note    Used for FRTTransceiver#printCommunicationsSummary
 */
#define FRTTRANSCEIVER_COMMTYPE2                            ("MULTISENDER QUEUE (READONLY)")
/*!
 * \brief   Describes an echo communicatation line, where a task set him self as his communication partner
 * \note    Used for FRTTransceiver#printCommunicationsSummary
 */
#define FRTTRANSCEIVER_COMMTYPE3                            ("ECHO")
/*!
 * \brief   When the address of the partner was not supplied
 * \note    Used by FRTTransceiver::printCommunicationsSummary
 */
#define FRTTRANSCEIVER_UNKNOWNADDRESS                       (0x00000000)
#endif

namespace FRTT {
    /*!
    * \brief         Enumeration is used to select a multi-sender-queue for different class methods
    * \details       Normally one would need to select a communication line by supplying a ::FRTTTaskHandle.
    *                A multi-sender-queue does not have a unique ::FRTTTaskHandle, so that another way of selecting a communication line
    *                had to be introduced.<br>
    * \note          There should be a maximum of #FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE enumerators. <br>
    *                Increasing #FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE also requires one to adjust the amount of eMultiSenderQueue enumerators to match #FRTTRANSCEIVER_MAXELEMENTSIZEONQUEUE
    *                
    */
    enum class eMultiSenderQueue : int8_t 
    {
        eNOMULTIQSELECTED   = -1,           /*!< It is just a filler enumerator if one wants to use a FRTTTaskHandle for selecting a communication line*/
        eMULTISENDERQ0,                     /*!< Select the FIRST multi-sender-queue added with FRTTransceiver::addMultiSenderPartner()*/
        eMULTISENDERQ1,                     /*!< Select the SECOND multi-sender-queue added with FRTTransceiver::addMultiSenderPartner()*/
        eMULTISENDERQ2,                     /*!< Select the THIRD multi-sender-queue added with FRTTransceiver::addMultiSenderPartner()*/
        eMULTISENDERQ3,                     /*!< Select the FOURTH multi-sender-queue added with FRTTransceiver::addMultiSenderPartner()*/
    };
}
#endif