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

#ifndef __NEMSGFILERESUME_H
#define __NEMSGFILERESUME_H

#include "nemsgconsts.h"

typedef struct
{
    unsigned long msgType;
    unsigned long msgPosition;
    unsigned long msgLength;
} NEMSGFILERESUME_HEADER;

class nemsgFileResume
{
  public:
    nemsgFileResume(ncSocket *sock);
    ~nemsgFileResume();

    /*
      sends an NE_MSG_FILE_RESUME message over the socket
      requesting the file starting at the specified position.
      returns 0 on success; 1 on error
    */
    int send(char *filename, unsigned long position);

    /*
      reads an NE_MSG_FILE_RESUME message over the socket and places
      up to maxlen bytes into the provided filename buffer.
      returns 0 on success; 1 on error
    */
    int recv(unsigned long *position, 
             char *filename,
             unsigned long maxlen);

  private:
    ncSocket *m_sock;
};

#endif /* __NEMSGFILERESUME_H */
