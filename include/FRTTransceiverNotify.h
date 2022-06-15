#ifndef FRTTRANSCEIVERNOTIFY_H
#define FRTTRANSCEIVERNOTIFY_H

/*!
 * \file        FRTTransceiverNotify.h
 * \brief       Extension for the FreeRTOS task notification api
 * \author      Xhemail Ramabaja (x.ramabaja@outlook.de)
 * \version     v1.1.0
 * \copyright   Copyright 2022 Xhemail Ramabaja
 *
 */
#include <stdint.h>

namespace FRTT {
    /*!
    * \brief         ------
    * \details       ------
    * \note          ------
    *                
    */
    enum class eFRTTNotifyActions : uint8_t
    {
        e_CLEARCOUNTONEXIT = 0,                 /*!< Used for FRTT::FRTTransceiver::NotifyReceiveBasic(). Tells FreeRTOS to clear the tasks notification value to zero      */
        e_DECREMENTCOUNTONEXIT,                 /*!< Used for FRTT::FRTTransceiver::NotifyReceiveBasic(). Tells FreeRTOS to decrement the tasks notification value          */
        e_NoAction,                             /*!< Used for FRTT::FRTTransceiver::NotifyExtended(). Tells FreeRTOS to set the partner tasks notification state to pending. u32 bit notification mask not used*/
        e_SetBits,                              /*!< Used for FRTT::FRTTransceiver::NotifyExtended(). Tells FreeRTOS to do a bitwise OR between the current notfication value of the task and the u32 notification mask*/
        e_Increment,                            /*!< Used for FRTT::FRTTransceiver::NotifyExtended(). Tells FreeRTOS to increment the partner tasks notification value. u32 bit notification mask not used.*/
        e_SetValueWithOverwrite,                /*!< Used for FRTT::FRTTransceiver::NotifyExtended(). Tells FreeRTOS to set the u32 bit notification value, regardless of whether the partner task has a notification pending */                    
        e_SetValueWithoutOverwrite,             /*!< Used for FRTT::FRTTransceiver::NotifyExtended(). Tells FreeRTOS to set the u32 bit notification value only if the task has no notification pending */
    };
}
#endif