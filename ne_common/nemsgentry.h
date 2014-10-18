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

#ifndef __NEMSGENTRY_H
#define __NEMSGENTRY_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgLength;
    unsigned long filesize;
} NEMSGENTRY_HEADER;

class nemsgEntry : public nemsgCommon
{
  public:
    nemsgEntry(ncSocket *sock);
    ~nemsgEntry();
   
    /*
      sends an NE_MSG_ENTRY message with the specified
      filename and filesize (in bytes).
      returns 0 on success; 1 on failure.
    */
    int send(char *filename, unsigned long filesize);

    /*
      copies an NE_MSG_ENTRY message from the socket into
      m_buf.  returns 0 on success; 1 on error
    */
    virtual int recv();

    /*
      receives an NE_MSG_ENTRY message and fills in the
      filename given, as well as the filesize (in bytes).
      fills in up to maxlen chars of filename.  
      returns 0 on success; 1 on failure
    */
    int recv(char *filename, int maxlen, unsigned long *filesize);

    /*
      formats data already read (i.e. stored in the m_buf)
      and copies it into the passed in arguments
      returns 0 on success; 1 on error
    */ 
    int format(char *filename, int maxlen, unsigned long *filesize);
};

#endif /* __NEMSGENTRY_H */
