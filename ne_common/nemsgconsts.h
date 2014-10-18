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

#ifndef __NEMSGCONSTS_H
#define __NEMSGCONSTS_H

/* macros used in neshare implementation */
#ifndef MIN
#define MIN(a,b)           ((a < b) ? a : b)
#endif

#define MEGABYTE                    0x100000
#define BYTES_TO_MBYTES(x) (int)(x/MEGABYTE)

const unsigned char INVALID_MD5_CHECKSUM[16] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

/* misc constants used in neshare implementation */
const unsigned long NESERVER_MAX_NAME_LEN            = 0x00000100;
const unsigned long INVALID_FILE_ID                  = 0xFFFFFFFF;

const unsigned long NE_MSG_MAX_DATA_LEN              = 0x00000100;
const unsigned long NE_MSG_MAX_GREETING_LEN          = 0x00002000;
const unsigned long NE_MSG_MAX_ERROR_LEN             = 0x00000040;
const unsigned long NE_MSG_MAX_RAW_DATA_LEN          = 0x00000200;
const unsigned long NE_MSG_MAX_NUM_KEYWORDS          = 0x00000010;
const unsigned long NE_MSG_MAX_KEYWORD_LEN           = 0x00000018;
const unsigned long NE_MAX_BUF_READ_LEN              = 0x00001000;

/*
  NOTE:
  The following messages are sent to the user for various reasons.
  They *should* be configurable to allow customization of the server.
  These are default strings used in case they are not configured.
*/
#define NE_CONFIG_GOODBYE_MESSAGE \
"Thanks for visiting.  Please come again.\n\n"

#define NE_CONFIG_GREETING_MESSAGE \
"Welcome to our NEshare Server!\n\n"

#define NE_CONFIG_LOGIN_FAILED_MESSAGE \
"Too many users connected. Please try again later.\n\n"

#define NE_CONFIG_INVALID_MESSAGE_TYPE \
"Invalid message type.  Protocol violation.\n\n"


/* valid connection speeds */
const unsigned long NEUSER_CONNECTION_14_4           =          0;
const unsigned long NEUSER_CONNECTION_28_8           =          1;
const unsigned long NEUSER_CONNECTION_33_6           =          2;
const unsigned long NEUSER_CONNECTION_64_K           =          3;
const unsigned long NEUSER_CONNECTION_128_K          =          4;
const unsigned long NEUSER_CONNECTION_CABLE          =          5;
const unsigned long NEUSER_CONNECTION_DSL            =          6;
const unsigned long NEUSER_CONNECTION_T1             =          7;
const unsigned long NEUSER_CONNECTION_T3             =          8;

/* all valid network message types for neshare */
/* client-to-server/server-to-client messages */
const unsigned long NE_MSG_ERROR                     = 0x00000001;
const unsigned long NE_MSG_LOGIN                     = 0x00000002;
const unsigned long NE_MSG_LOGIN_ACK                 = 0x00000003;
const unsigned long NE_MSG_DISCONNECT                = 0x00000004;
const unsigned long NE_MSG_DISCONNECT_ACK            = 0x00000005;
const unsigned long NE_MSG_LOGIN_FAILED              = 0x00000006;
const unsigned long NE_MSG_ENTRY_SET_START           = 0x00000007;
const unsigned long NE_MSG_ENTRY                     = 0x00000008; 
const unsigned long NE_MSG_ENTRY_SET_END             = 0x00000009;
const unsigned long NE_MSG_PING                      = 0x00000020;
const unsigned long NE_MSG_PONG                      = 0x00000021;
const unsigned long NE_MSG_STATUS                    = 0x00000022;
const unsigned long NE_MSG_FORCED_DISCONNECT         = 0x00000023;
const unsigned long NE_MSG_SEARCH_QUERY              = 0x00000030;
const unsigned long NE_MSG_SEARCH_KEYWORD            = 0x00000031;
const unsigned long NE_MSG_SEARCH_RESULTS            = 0x00000032;
const unsigned long NE_MSG_QUERY_RESULT              = 0x00000033;
const unsigned long NE_MSG_PUSH_REQUEST              = 0x00000043;
const unsigned long NE_MSG_PUSH_REQUEST_ACK          = 0x00000044;

/* peer-to-peer messages */
const unsigned long NE_MSG_PEER_ERROR                = 0x00010001;
const unsigned long NE_MSG_PEER_LOGIN                = 0x00010002;
const unsigned long NE_MSG_PEER_LOGIN_ACK            = 0x00010003;
const unsigned long NE_MSG_PEER_DISCONNECT           = 0x00010004;
const unsigned long NE_MSG_PEER_DISCONNECT_ACK       = 0x00010005;
const unsigned long NE_MSG_PEER_LOGIN_FAILED         = 0x00010006;
const unsigned long NE_MSG_FILE_REQUEST              = 0x00010020;
const unsigned long NE_MSG_FILE_REQUEST_ACK          = 0x00010021;
const unsigned long NE_MSG_FILE_DATA_SEND            = 0x00010022;
const unsigned long NE_MSG_FILE_DATA_SEND_END        = 0x00010023;
const unsigned long NE_MSG_FILE_DATA_CANCEL          = 0x00010024;
const unsigned long NE_MSG_FILE_RESUME               = 0x00010025;
const unsigned long NE_MSG_FILE_RESUME_ACK           = 0x00010026;

#endif /* __NEMSGCONSTS_H */
