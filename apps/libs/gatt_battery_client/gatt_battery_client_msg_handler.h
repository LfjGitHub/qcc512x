/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/* Part of 6.3.0 */

#ifndef GATT_BATTERY_CLIENT_MSG_HANDLER_H_
#define GATT_BATTERY_CLIENT_MSG_HANDLER_H_

/***************************************************************************
NAME
    batteryClientMsgHandler

DESCRIPTION
    Handler for external messages sent to the library in the client role.
*/
void batteryClientMsgHandler(Task task, MessageId id, Message payload);


#endif
