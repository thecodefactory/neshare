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

#ifndef __NEMSGFILEDATASEND_H
#define __NEMSGFILEDATASEND_H

typedef struct
{
    unsigned long msgType;
    unsigned long msgFileID;
    unsigned long msgLength;
} NEMSGFILEDATASEND_HEADER;

class nemsgFileDataSend
{
  public:
    nemsgFileDataSend(ncSocket *sock);
    ~nemsgFileDataSend();

    /*
      sends an NE_MSG_FILE_DATA_SEND message over the socket.
      NOTE: There is no restriction on the length of the
      data to send in this call.  dataSize bytes of data
      will send (or at least be attempted).
      returns 0 on success; 1 on failure
    */
    int send(unsigned long fileID,
             unsigned long dataSize,
             void *data);

    /*
      reads an NE_MSG_FILE_DATA_SEND message over the socket and places
      up to maxlen bytes into the provided data buffer.  The fileID
      and the dataSize will be filled in.
      returns 0 on success; 1 on failure
    */
    int recv(unsigned long *fileID,
             unsigned long *dataSize, 
             void *data,
             int maxlen);

    /*
      sets the socket to either send or receive on.
      this was added for efficiency regarding how this message can
      be used as illustrated in nePeerUploadManager::sendPeerData()
    */
    void setSocket(ncSocket *sock);

    /* 
       NOTE: if the following methods are to be used, do not use
       the recv call because the following methods remove the data
       from the socket buffers.  These are not peeked reads.
      
       More importantly, they MUST be called in the order that they
       are laid out in the message (defined in the protocol specs).
      
       The reason these methods were designed for this class in particular
       is to avoid copying incoming file data multiple times.  While this
       does break the 'mostly clean' abstraction model of these message
       classes, something like this is a small price to pay for the added
       performance and ease. Any better ideas?
       -N.M.
    */

    /* returns the message type (for verification) */
    unsigned long getMessageType();

    /* returns the fileID */
    unsigned long getFileID();

    /* returns the length of the following data */
    unsigned long getDataLength();

    /* 
       reads maxlen bytes into buffer. 
       returns 0 on success; 1 on failure
    */
    int readData(void *buffer,unsigned long maxlen);

  private:
    unsigned long m_msgType;
    unsigned long m_fileID;
    unsigned long m_dataLength;

    ncSocket *m_sock;
};

#endif /* __NEMSGFILEDATASEND_H */
