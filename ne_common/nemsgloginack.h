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

#ifndef __NEMSGLOGINACK_H
#define __NEMSGLOGINACK_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgLength;
} NEMSGLOGINACK_HEADER;

class nemsgLoginAck : public nemsgCommon
{
  public:
    nemsgLoginAck(ncSocket *sock);
    ~nemsgLoginAck();
   
    /*
      sends an NE_MSG_LOGIN_ACK message over the socket.
      returns 0 on success; 1 on failure
    */
    int send(char *loginackmsg);

    /*
      copies an NE_MSG_LOGIN_ACK message from the socket
      into m_buf.  returns 0 on success; 1 on error
    */
    virtual int recv();

    /*
      reads an NE_MSG_LOGIN_ACK message from the socket and places
      up to maxlen bytes into the provided loginackmsg buffer.
      returns 0 on success; 1 on failure
    */
    int recv(char *loginackmsg, int maxlen);
   
    /*
      formats data already read (i.e. stored in the m_buf)
      and copies it into the passed in arguments
      returns 0 on success; 1 on error
    */
    int format(char *loginackmsg, int maxlen);
};

#endif /* __NEMSGLOGINACK_H */
