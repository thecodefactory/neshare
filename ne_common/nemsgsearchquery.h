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

#ifndef __NEMSGSEARCHQUERY_H
#define __NEMSGSEARCHQUERY_H

typedef struct
{
    unsigned long msgType;
    unsigned long numKeywords;
} NE_MSG_SQ_HEADER;

typedef struct
{
    unsigned long typeFlag;
    unsigned long keywordLen;
} NE_MSG_SQ_FIELD;

/*
  NOTE: No "format" method is used for this message.
  Instead, if recv() is called, the message will be
  copied into m_buf.  When the getNumKeywords and
  getNextKeyword methods are called, if m_buf contains
  data, the output data will be copied from m_buf;
  otherwise, the output data will be read from the socket.
*/

class nemsgSearchQuery : public nemsgCommon
{
  public:
    nemsgSearchQuery(ncSocket *sock);
    ~nemsgSearchQuery();

    /*
      sends an NE_MSG_SEARCH_QUERY message over the socket.
      all keywords in the keyword string array are sent
      with the corresponding typeflag based on position
      in the two arrays.  Both arrays must be the same size.
      No checking is done for this, so your app will crash
      if you call this the wrong way.

      simple code example (of typical usage):
      unsigned long numKeywords = 2
      unsigned long typeFlags[2] = {0,0};
      char *keywords[] = {"foo","bar"};
      nemsgSearchQuery::send(numKeywords,typeFlags,keywords);

      returns 0 on success; 1 on failure
    */
    int send(unsigned long numKeywords, 
             unsigned long *typeFlags,
             char **keywords);

    /*
      copies an NE_MSG_SEARCH_QUERY message from the socket into
      m_buf.  returns 0 on success; 1 on error
    */
    virtual int recv();

    /*
      places the number of entries into the provided numEntries argument.
      returns 0 on success; 1 on failure
    */
    int getNumKeywords(unsigned long *numKeywords);

    /* 
       retrieves the next keyword field block.
       returns 0 on success; 1 on failure
    */
    int getNextKeyword(unsigned long *typeFlag,
                       char *keyword,
                       unsigned long maxlen);

  private:
    char *m_bufPtr;
    unsigned long m_numKeywords;
};

#endif /* __NEMSGSEARCHQUERY_H */
