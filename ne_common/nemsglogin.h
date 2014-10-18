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

#ifndef __NEMSGLOGIN_H
#define __NEMSGLOGIN_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgConnection;
    unsigned long msgFirewallStatus;
    unsigned long msgControlPort;
} NEMSGLOGIN_HEADER;

class nemsgLogin : public nemsgCommon
{
  public:
    nemsgLogin(ncSocket *sock);
    ~nemsgLogin();

    /*
      sends an NE_MSG_LOGIN message over the socket, sending
      the connection speed specified in connection.
      (see nemsgconsts.h for valid values)
      If the client is behind a firewall, the firewall
      status should be 1; otherwise 0.
      returns 0 on success; 1 on error
    */
    int send(unsigned long connection, 
             unsigned long firewallStatus,
             unsigned long controlPort);

    /*
      copies an NE_MSG_LOGIN message from the socket into
      m_buf.  returns 0 on success; 1 on error
    */
    virtual int recv();

    /*
      reads an NE_MSG_LOGIN message over the socket,
      placing the connection speed into connection.
      returns 0 on success; 1 on error
    */
    int recv(unsigned long *connection,
             unsigned long *firewallStatus,
             unsigned long *controlPort);

    /*
      formats data already read (i.e. stored in the m_buf)
      and copies it into the passed in arguments
      returns 0 on success; 1 on error
    */ 
    int format(unsigned long *connection,
               unsigned long *firewallStatus,
               unsigned long *controlPort);
};

#endif /* __NEMSGLOGIN_H */
