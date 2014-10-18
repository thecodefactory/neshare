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

#include "necommonheaders.h"

nemsgSearchQuery::nemsgSearchQuery(ncSocket *sock) : nemsgCommon(sock)
{
    m_bufPtr = (char *)0;
    m_numKeywords = 0;
}

nemsgSearchQuery::~nemsgSearchQuery()
{
    m_bufPtr = (char *)0;
    m_numKeywords = 0;
}

int nemsgSearchQuery::send(unsigned long numKeywords,
                           unsigned long *typeFlags,
                           char **keywords)
{
    int ret = 0;
    NE_MSG_SQ_HEADER header;
    NE_MSG_SQ_FIELD field;
    unsigned long keywordLen = 0;
    std::string newKeyword;

    header.msgType = htonl(NE_MSG_SEARCH_QUERY);
    header.numKeywords = htonl(numKeywords);

    if (typeFlags && keywords &&
        m_sock->writeData((void *)&header,
                          sizeof(NE_MSG_SQ_HEADER)) == NC_OK)
    {
        /* write out all fields and then the keywords */
        for(unsigned long i = 0; i < numKeywords; i++)
        {
            newKeyword = "";
            /*
              NOTE: I'm arbitrarily saying a keyword can be NO LONGER
              than NE_MSG_MAX_DATA_LEN/4, even though this isn't in
              the protocol spec.  This is an arbitrary implementation
              limitation
            */
            keywordLen = (keywords[i] ?
                          MIN(strlen(keywords[i]),(NE_MSG_MAX_DATA_LEN >> 2)) :
                          0);
            if (keywordLen)
            {
                /* make sure the keyword is in all caps */
                newKeyword = neUtils::capitalizeStr(keywords[i]);
                assert(keywordLen == newKeyword.length());
            }
            field.typeFlag = htonl(typeFlags[i]);
            field.keywordLen = htonl(keywordLen);

            if ((m_sock->writeData((void *)&field,
                                   sizeof(NE_MSG_SQ_FIELD)) == NC_OK) &&
                (m_sock->writeData((void *)newKeyword.c_str(),
                                   keywordLen) == NC_OK))
            {
                ++ret;
            }
        }
        ret = (((unsigned long)ret == numKeywords) ? 0 : 1);
    }
    return ret;
}

int nemsgSearchQuery::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = 0;
    unsigned long numKeywords = 0;
    unsigned long keywordLen = 0;
    unsigned long keywordCount = 0;

    if (m_sock->readData((void *)ptr,2*sizeof(unsigned long)) == NC_OK)
    {
        tokenType = ntohl(*((unsigned long *)ptr));
        if (tokenType == NE_MSG_SEARCH_QUERY)
        {
            /* skip over tokenType */
            ptr += sizeof(unsigned long);

            numKeywords = ntohl(*((unsigned long *)ptr));
            if (numKeywords <= NE_MSG_MAX_NUM_KEYWORDS)
            {
                /* skip over numKeywords */
                ptr += sizeof(unsigned long);

                for(unsigned long i = 0; i < numKeywords; i++)
                {
                    if (m_sock->readData((void *)ptr,
                                         2*sizeof(unsigned long)) == NC_OK)
                    {
                        /* skip over typeFlag */
                        ptr += sizeof(unsigned long);

                        keywordLen = ntohl(*((unsigned long *)ptr));
                        if (keywordLen <= NE_MSG_MAX_KEYWORD_LEN)
                        {
                            /* skip over keywordLen */
                            ptr += sizeof(unsigned long);
                     
                            if (m_sock->readData((void *)ptr,keywordLen) == NC_OK)
                            {
                                ptr += keywordLen;
                                keywordCount++;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
    if (numKeywords == keywordCount)
    {
        m_bufPtr = m_buf;
        ret = 0;
    }
    return ret;
}

int nemsgSearchQuery::getNumKeywords(unsigned long *numKeywords)
{
    int ret = 1;

    assert(numKeywords);

    if (m_numKeywords)
    {
        return m_numKeywords;
    }
    else if (m_bufPtr == m_buf)
    {
        assert(ntohl(*((unsigned long *)m_bufPtr)) == NE_MSG_SEARCH_QUERY);
        m_bufPtr += sizeof(unsigned long);

        *numKeywords = ntohl(*((unsigned long *)m_bufPtr));
        m_bufPtr += sizeof(unsigned long);

        m_numKeywords = *numKeywords;
        if (m_numKeywords && (m_numKeywords <= NE_MSG_MAX_NUM_KEYWORDS))
        {
            ret = 0;
        }
    }
    else
    {
        NE_MSG_SQ_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(msgHeader));
      
        if (m_sock->readData((void *)&msgHeader,
                             sizeof(msgHeader)) == NC_OK)
        {
            msgHeader.msgType = ntohl(msgHeader.msgType);
            msgHeader.numKeywords = ntohl(msgHeader.numKeywords);
         
            if (msgHeader.msgType == NE_MSG_SEARCH_QUERY)
            {
                *numKeywords = msgHeader.numKeywords;
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgSearchQuery::getNextKeyword(unsigned long *typeFlag,
                                     char *keyword,
                                     unsigned long maxlen)
{
    int ret = 1;
    unsigned long keywordLen = MIN(NE_MSG_MAX_KEYWORD_LEN,maxlen);

    assert(typeFlag);
    assert(keyword);

    if (m_bufPtr && m_numKeywords)
    {
        *typeFlag = *((unsigned long *)m_bufPtr);

        m_bufPtr += sizeof(unsigned long);
        keywordLen = MIN(keywordLen,ntohl(*((unsigned long *)m_bufPtr)));

        if (keywordLen <= NE_MSG_MAX_KEYWORD_LEN)
        {
            m_bufPtr += sizeof(unsigned long);
            memcpy(keyword,m_bufPtr,keywordLen);
            m_bufPtr += keywordLen;

            keyword[keywordLen] = '\0';
            ret = 0;
        }
    }
    else
    {
        NE_MSG_SQ_FIELD msgField;
        memset(&msgField,0,sizeof(msgField));

        if (m_sock->readData((void *)&msgField,sizeof(msgField)) == NC_OK)
        {
            msgField.typeFlag = ntohl(msgField.typeFlag);
            msgField.keywordLen = ntohl(msgField.keywordLen);

            *typeFlag = msgField.typeFlag;
            keywordLen = MIN(keywordLen,msgField.keywordLen);

            if (m_sock->readData((void *)keyword,keywordLen) == NC_OK)
            {
                keyword[keywordLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
