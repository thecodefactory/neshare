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

#ifndef __NEMSGFILEREQUESTACK_H
#define __NEMSGFILEREQUESTACK_H

typedef struct
{
    unsigned long msgType;
    unsigned long fileSendID;
    unsigned long maxBlockSize;
    unsigned long filesize;
    unsigned char md5checksum[16];
    unsigned long msgFilenameLen;
} NEMSGFILEREQUESTACK_HEADER;

class nemsgFileRequestAck
{
  public:
    nemsgFileRequestAck(ncSocket *sock);
    ~nemsgFileRequestAck();

    /*
      sends an NE_MSG_FILE_REQUEST_ACK message with the specified
      file send ID and max block size along with the md5checksum
      and the filename.  returns 0 on success; 1 on failure.
    */
    int send(unsigned long fileSendID, 
             unsigned long maxBlockSize,
             unsigned long filesize,
             unsigned char md5checksum[16],
             char *filename);

    /*
      receives an NE_MSG_FILE_REQUEST_ACK message and fills in
      the file send ID, the max block size, the md5checksum,
      as well as the filename up to maxlen bytes.
      returns 0 on success; 1 on failure
    */
    int recv(unsigned long *fileSendID, 
             unsigned long *maxBlockSize,
             unsigned long *filesize,
             unsigned char md5checksum[16],
             char *filename, 
             unsigned long maxlen);

  private:
    ncSocket *m_sock;
};

#endif /* __NEMSGFILEREQUESTACK_H */
