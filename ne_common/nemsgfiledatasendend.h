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

#ifndef __NEMSGFILEDATASENDEND_H
#define __NEMSGFILEDATASENDEND_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgFileID;
} NEMSGFILEDATASENDEND_HEADER;

class nemsgFileDataSendEnd
{
  public:
    nemsgFileDataSendEnd(ncSocket *sock);
    ~nemsgFileDataSendEnd();

    /*
      sends an NE_MSG_FILE_DATA_SEND_END message over the socket for
      the specified fileID (to note the end of data).
      returns 0 on success; 1 on failure
    */
    int send(unsigned long fileID);

    /*
      reads an NE_MSG_FILE_DATA_SEND_END message over the socket 
      and places the appropriate fileID into the provided 
      fileID argument.  returns 0 on success; 1 on failure
    */
    int recv(unsigned long *fileID);

  private:
    ncSocket *m_sock;
};

#endif /* __NEMSGFILEDATASENDEND_H */
