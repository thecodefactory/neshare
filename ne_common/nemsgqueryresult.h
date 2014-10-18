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

#ifndef __NEMSGQUERYRESULT_H
#define __NEMSGQUERYRESULT_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgIpAddr;
    unsigned long msgConnection;
    unsigned long msgFirewallStatus;
    unsigned long msgFilesize;
    unsigned long msgControlPort;
    unsigned long msgID;
    unsigned long msgResultLen;
} NEMSGQUERYRESULT_HEADER;

class nemsgQueryResult
{
  public:
    nemsgQueryResult(ncSocket *sock);
    ~nemsgQueryResult();

    /*
      sends an NE_MSG_QUERY_RESULT message with the specified
      parameters.  returns 0 on success; 1 on failure.
    */
    int send(unsigned long ipAddr,
             unsigned long connection,
             unsigned long firewallStatus,
             unsigned long filesize,
             unsigned long controlPort,
             unsigned long id,
             char *result);

    /*
      receives an NE_MSG_QUERY_RESULT message and fills
      in the given parameters.
      returns 0 on success; 1 on failure
    */
    int recv(unsigned long *ipAddr, 
             unsigned long *connection, 
             unsigned long *firewallStatus,
             unsigned long *filesize,
             unsigned long *controlPort,
             unsigned long *id,
             char *result, 
             int maxresultlen);

  private:
    ncSocket *m_sock;
};

#endif /* __NEMSGQUERYRESULT_H */
