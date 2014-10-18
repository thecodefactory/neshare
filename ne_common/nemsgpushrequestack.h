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

#ifndef __NEMSGPUSHREQUESTACK_H
#define __NEMSGPUSHREQUESTACK_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgLength;
} NEMSGPUSHREQUESTACK_HEADER;

class nemsgPushRequestAck
{
  public:
    nemsgPushRequestAck(ncSocket *sock);
    ~nemsgPushRequestAck();

    /*
      sends an NE_MSG_PUSH_REQUEST message over the socket.
      returns 0 on success; 1 on error
    */
    int send(char *filename);

    /*
      reads an NE_MSG_PUSH_REQUEST message over the socket and places
      up to maxlen bytes into the provided filename buffer.
      returns 0 on success; 1 on error
    */
    int recv(char *filename, 
             unsigned long maxlen);

  private:
    ncSocket *m_sock;
};

#endif /* __NEMSGPUSHREQUESTACK_H */
