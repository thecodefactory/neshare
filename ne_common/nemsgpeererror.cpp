/*
  NEshare is a peer-to-peer file sharing toolkit.
  Copyright (C) 2001, 2002 Neill Miller
  This file is part of NEshare.
  
  NEshare is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  NEshare is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with NEshare; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "necommonheaders.h"

nemsgPeerError::nemsgPeerError(ncSocket *sock)
{
   m_sock = sock;
}

nemsgPeerError::~nemsgPeerError()
{
   m_sock = (ncSocket *)0;
}

int nemsgPeerError::send(char *peerErrorMsg)
{
   int ret = 1;
   int peerErrorMsgLen =
       (peerErrorMsg ? MIN(strlen(peerErrorMsg),NE_MSG_MAX_DATA_LEN) : 0);

   if (peerErrorMsgLen)
   {
       NEMSGPEERERROR_HEADER msgHeader;
       memset(&msgHeader,0,sizeof(NEMSGPEERERROR_HEADER));
       msgHeader.msgType = htonl(NE_MSG_PEER_ERROR);
       msgHeader.msgLength = htonl(peerErrorMsgLen);
   
       if ((m_sock->writeData((void *)&msgHeader,
                              sizeof(NEMSGPEERERROR_HEADER)) == NC_OK) &&
           (m_sock->writeData((void *)peerErrorMsg,peerErrorMsgLen) == NC_OK))
       {
           ret = 0;
       }
   }
   return ret;
}

int nemsgPeerError::recv(char *peerErrorMsg, int maxlen)
{
   int ret = 1;
   unsigned long peerErrorMsgLen = MIN((unsigned long)maxlen,
                                       NE_MSG_MAX_DATA_LEN-1);
   NEMSGPEERERROR_HEADER msgHeader;
   memset(&msgHeader,0,sizeof(NEMSGPEERERROR_HEADER));

   if (peerErrorMsg &&
       m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
   {
      if (ntohl(msgHeader.msgType) == NE_MSG_PEER_ERROR)
      {
         peerErrorMsgLen = MIN(peerErrorMsgLen,
                               ntohl(msgHeader.msgLength));

         if (m_sock->readData((void *)peerErrorMsg,
                              peerErrorMsgLen) == NC_OK)
         {
            peerErrorMsg[peerErrorMsgLen] = '\0';
            ret = 0;
         }
      }
   }
   return ret;
}
