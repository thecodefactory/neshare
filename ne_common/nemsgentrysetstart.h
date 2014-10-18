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

#ifndef __NEMSGENTRYSETSTART_H
#define __NEMSGENTRYSETSTART_H

typedef struct
{
    unsigned long msgType;
    unsigned long numEntries;
} NEMSGENTRYSETSTART_HEADER;

class nemsgEntrySetStart : public nemsgCommon
{
  public:
    nemsgEntrySetStart(ncSocket *sock);
    ~nemsgEntrySetStart();
   
    /*
      sends an NE_MSG_ENTRY_SET_START message over the socket.
      returns 0 on success; 1 on failure
    */
    int send(unsigned long numEntries);

    /*
      copies an NE_MSG_ERROR message from the socket into
      m_buf.  returns 0 on success; 1 on error
    */
    virtual int recv();

    /*
      reads an NE_MSG_ENTRY_SET_START message over the socket
      and places the number of entries into the provided
      numEntries argument.  returns 0 on success; 1 on failure
    */
    int recv(unsigned long *numEntries);

    /*
      formats data already read (i.e. stored in the m_buf)
      and copies it into the passed in arguments
      returns 0 on success; 1 on error
    */ 
    int format(unsigned long *numEntries);
};

#endif /* __NEMSGENTRYSETSTART_H */
