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

#ifndef __NEMSGCOMMON_H
#define __NEMSGCOMMON_H

/*
  The nemsgCommon data type is used for providing minimal required
  functionality from all other server based NEshare messages (for
  this specific implementation).  Server based messages include all
  messages that the server is capable of receiving from a connected
  client.  No other NEshare message are derived from nemsgCommon.

  The recv method with no arguments is used as the first step in a two
  part process if used at all.  If recv() is called, a message type
  is read from the socket and copied directly into the m_buf buffer.

  The data in the m_buf is unformatted data.  To retrieve specific
  fields from the m_buf, use the format methods.  The format methods
  take arguments specific to each message type and format the raw
  data and pass the relevant pieces out to the caller via the arguments.

  If the recv(...) methods of the derived classes are used directly,
  no additional copy is made, and the formatted data is passed directly
  to the caller via the arguments.

  In general, the recv(...) methods should be used.  The recv() and
  format options are available if there is a specific need for that
  kind of behaviour.  It can be advantageous because it allows a
  specific message type to be removed from the socket stream without
  necessarily being interpreted.
*/

class nemsgCommon
{
  public:
    nemsgCommon(ncSocket *sock)
    {
        m_sock = sock;
        memset(m_buf,0,NE_MSG_MAX_RAW_DATA_LEN);
    }

    virtual ~nemsgCommon()
    {
        m_sock = (ncSocket *)0;
    }

    ncSocket *getSocket()
    {
        return m_sock;
    }

    void setSocket(ncSocket *sock)
    {
        m_sock = sock;
    }

    /*
      a generic message specific method used for reading
      in the next available token and accompanying data
      as raw data into the m_buf.  should be used only in
      conjunction with the "format" methods of the specific
      msg types.  returns 0 on success; 1 on error
    */
    virtual int recv() = 0;

  protected:
    ncSocket *m_sock;
    char m_buf[NE_MSG_MAX_RAW_DATA_LEN];
};

#endif /* __NEMSGCOMMON_H */
