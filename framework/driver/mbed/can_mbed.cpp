/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
mbed Port: sgrove

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//#define DEBUG_WAR_CONSOLE_ON
//#define DEBUG_ERR_CONSOLE_ON

#include "can_mbed.h"
#include "canfestival.h"

volatile unsigned char msg_received = 0;
// initialize the CAN object from the mbed api

CAN CANopen(PA_11,PA_12);

unsigned char canInit(unsigned int bitrate)
/******************************************************************************
Initialize the hardware to send and receive CAN messages 
INPUT    bitrate        bitrate in kilobit
OUTPUT    1 if successful    
******************************************************************************/
{
      // make sure the requested baudrate is supported
    if (CANopen.frequency(bitrate*1000) == 0){
          return 0;
    }
      // desired baud was set
    return 1;
}

unsigned char canSend(CAN_PORT notused, Message *m)
/******************************************************************************
The driver send a CAN message passed from the CANopen stack
INPUT    CAN_PORT is not used (only 1 avaiable)
    Message *m pointer to message to send
OUTPUT    1 if  hardware -> CAN frame
******************************************************************************/
{
    // convert the message from a CANopen object to a mbed object
    CANMessage msg(m->cob_id, (char*)m->data, m->len, static_cast<CANType>(m->rtr), CANStandard);
    // make sure the message was sent
    if (CANopen.write(msg) == 0){
        return 0;                               
    }
    // message was sent
    return 1;
}

unsigned char canReceive(Message *m)
/******************************************************************************
The driver pass a received CAN message to the stack
INPUT    Message *m pointer to received CAN message
OUTPUT    1 if a message received
******************************************************************************/
{
    // object to store the last message
    CANMessage msg;
    // look if something has been rec'd
    if (CANopen.read(msg) == 0){
        return 0;
    }
    // conver the CANMessage object to a Message object
    m->cob_id = msg.id;
    m->len = msg.len;
    m->rtr = static_cast<UNS8>(msg.type);
    // clear erroneous data from the last use
    for (int i=0; i<=7; i++){
        if (i <= (msg.len-1))
            m->data[i] = msg.data[i];
        else
            m->data[i] = 0;
    }
    // messge processed
    return 1;
}

/***************************************************************************/
unsigned char canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
    // not sure how baud is passed as a char* yet
    return 0;
}

