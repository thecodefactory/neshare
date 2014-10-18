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

extern int errno;

/* all common util functions are defined in namespace "neUtils" */
namespace neUtils
{
    unsigned long peekMessage(ncSocket *sock)
    {
        unsigned long msgType = 0;
        unsigned long peekAttempts = 0;

        if (sock)
        {
            /*
              do a peek read from socket of the next 4 bytes and
              return them if any bytes are available within a
              preset amount of time.
            */
            while(peekAttempts++ < 5)
            {
                if (recv(sock->getSockfd(),(void *)&msgType,
                         sizeof(unsigned long),MSG_PEEK) ==
                    sizeof(unsigned long))
                {
                    msgType = ntohl(msgType);
                    break;
                }
                usleep(10);
            }
        }
        return msgType;
    }

    /*
      this function formats the filename by stripping off any leading
      path components separated by a '/'. It takes an NEshare encoded 
      filename such as neshare://foo/bar/baz.txt thus it is guaranteed 
      to have at least one leading '/' character if used properly.
    */
    char *getFormattedFilename(char *filename)
    {
        char *ptr = filename+strlen(filename);
        while(*ptr-- != '/');
        return ptr+2;
    }

    /*
      similar to getFormattedFilename, except is safer since this
      doesn't assume that a '/' or '\\' character must be present 
    */
    std::string getFormattedOutputFilename(char *filename)
    {
        char *ptr = (char *)0;
        int filenameLen = strlen(filename);
      
        /*
          starting at the end of the filename, scan backwards
          and stop at the first "/" or "\\" character if there
          are any present in the filename.
        */
        for(ptr = filename+filenameLen; ptr != filename; ptr--)
        {
            if ((*ptr == (char)'/') || (*ptr == (char)'\\'))
            {
                /*
                  if we found a slash, move ahead
                  past it and stop scanning
                */
                ptr++;
                break;
            }
        }
        return std::string(ptr);
    }

    /* 
       similar to getFormattedFilename, but allows substitution of
       newChar wherever oldChar occurs in the filename.  Useful
       for converting spaces to underscores or things along the same
       lines.  Note that this does modify the incoming filename.
    */
    char *getFormattedFilenameWithSubstitute(char *filename,
                                             char oldChar,
                                             char newChar)
    {
        char *ptr = filename+strlen(filename);
        while(*ptr != '/')
        {
            if (*ptr == oldChar)
            {
                *ptr = newChar;
            }
            --ptr;
        }
        return ++ptr;
    }
   
    char *ipToStr(unsigned long ip)
    {
        struct in_addr inAddr;
        memset(&inAddr,0,sizeof(inAddr));
        inAddr.s_addr = ip;
        return inet_ntoa(inAddr);
    }
   
    int computeMD5Checksum(char *filename, unsigned char digest[16])
    {
        /* 
           given a filename, fills in a 16 byte md5 checksum into
           the specified digest argument;
           returns 0 on success; 1 if the file cannot be opened
        */
        unsigned short int ret = 1;
        unsigned char buf[8192];
        FILE *fd = (FILE *)0;
        md5_state_t md5ctx;
        unsigned long currentRead = 0;

        if ((fd = fopen(filename,"r")) != (FILE *)0)
        {
            md5_init(&md5ctx);
         
            while((currentRead =
                   fread((void *)buf,sizeof(unsigned char),8192,fd)) > 0)
            {
                md5_append(&md5ctx,
                           const_cast<unsigned char *>(buf),
                           (int)currentRead);
            }
            fclose(fd);
            md5_finish(&md5ctx,digest);
            ret = 0;
        }
        else
        {
            eprintf("neUtils::computeMD5Checksum | Cannot open %s "
                    "for reading (errno = %d)\n",filename,errno);
        }
        return ret;
    }

    /*
      this method was developed because the the computeMD5Checksum 
      method above is *extremely* slow on large files
    */
    int computeQuickMD5Checksum(char *filename, unsigned char digest[16])
    {
        /* 
           given a filename, fills in a 16 byte md5 checksum into
           the specified digest argument;
           returns 0 on success; 1 if the file cannot be accessed
           properly
         
           this is "quick" because it only computes the checksum on
           several pieces of the file instead of the entire file.
        */
        unsigned short int ret = 1;
        unsigned char buf[8192];
        FILE *fd = (FILE *)0;
        md5_state_t md5ctx;
        unsigned long currentRead = 0;
        unsigned long posIncrement = 0;

        struct stat statbuf;
        memset(&statbuf,0,sizeof(statbuf));

        if (stat(filename,&statbuf) == 0)
        {
            /*
              at most we're checking 256 blocks of 4096 bytes in
              the file (1MB of scattered data should suffice as
              a quick CRC)
            */
            posIncrement = (unsigned long)(statbuf.st_size/256);
            if ((fd = fopen(filename,"r")) != (FILE *)0)
            {
                md5_init(&md5ctx);

                while((currentRead =
                       fread((void *)buf,
                             sizeof(unsigned char),4096,fd)) > 0)
                {
                    md5_append(&md5ctx,
                               const_cast<unsigned char *>(buf),
                               (int)currentRead);
                    fseek(fd,posIncrement,SEEK_CUR);
                }
                fclose(fd);
                md5_finish(&md5ctx,digest);
                ret = 0;
            }
            else
            {
                eprintf("neUtils::computeQuickMD5Checksum | Cannot "
                        "open %s for reading (errno = %d)\n",
                        filename,errno);
            }
        }
        else
        {
            eprintf("neUtils::computeQuickMD5Checksum | Cannot stat %s "
                    "(errno = %d)\n",filename,errno);

        }
        return ret;
    }

    /* 
       this function returns a string based on the
       index specified for determining the connection speed.
       The values are from the NEshare protocol documentation.
    */
    char *getConnectionSpeedString(int connectionSpeed)
    {
        static char *connectionSpeeds[] = 
            {
                "14.4Kbps","28.8Kbps","33.6Kbps",
                "56.6Kbps","64Kbps","128Kbps",
                "Cable","DSL","T1","T3+","Unknown"
            };
        return (((connectionSpeed > -1) && (connectionSpeed < 10)) ?
                connectionSpeeds[connectionSpeed] : 
                connectionSpeeds[10]);
    }

    /* 
       this function returns a numeric value based on the
       string specified for determining the connection speed.
       The values are from the NEshare protocol documentation.
    */
    int getConnectionSpeedValue(char *connectionSpeed)
    {
        if (strcmp(connectionSpeed,"14.4Kbps") == 0)
        {
            return 0;
        }
        else if (strcmp(connectionSpeed,"28.8Kbps") == 0)
        {
            return 1;
        }
        else if (strcmp(connectionSpeed,"33.6Kbps") == 0)
        {
            return 2;
        }
        else if (strcmp(connectionSpeed,"56.6Kbps") == 0)
        {
            return 3;
        }
        else if (strcmp(connectionSpeed,"64Kbps") == 0)
        {
            return 4;
        }
        else if (strcmp(connectionSpeed,"128Kbps") == 0)
        {
            return 5;
        }
        else if (strcmp(connectionSpeed,"Cable") == 0)
        {
            return 6;
        }
        else if (strcmp(connectionSpeed,"DSL") == 0)
        {
            return 7;
        }
        else if (strcmp(connectionSpeed,"T1") == 0)
        {
            return 8;
        }
        else if (strcmp(connectionSpeed,"T3+") == 0)
        {
            return 9;
        }
        else
        {
            return 10;
        }
    }

    void discardSocketData(ncSocket *socket, int numBytes)
    {
        char buf[512];
        int bytesLeft = numBytes;
        int bytesRead = 0;
        int bytesToRead = 0;

        if (socket && numBytes)
        {
            while(bytesLeft > -1)
            {
                bytesRead = 0;
                bytesToRead = MIN(512,(unsigned long)numBytes);
                bytesToRead = MIN(bytesToRead,bytesLeft);

                if ((bytesToRead == 0) ||
                    (socket->readData(buf,bytesToRead,&bytesRead) ==
                     NC_FAILED))
                {
                    break;
                }
                bytesLeft -= bytesRead;
            }
        }
    }

    void getFormattedTime(char *outstr)
    {
        if (outstr)
        {
            struct timeval tv = {0,0};
            struct tm *ptm = 0;

            gettimeofday(&tv, 0);
            ptm = localtime((time_t *)&tv.tv_sec);

            snprintf(outstr, 16, "%02d:%02d:%02d", 
                     ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        }
    }

    std::string msgNumToStr(unsigned long msgType)
    {
        std::string ret;

        /*
          this is not complete; besides, a msgType to char *
          lookup table would be more efficient. This is only
          used for logging/print messages at the moment.
        */
        switch(msgType)
        {
            case NE_MSG_ERROR:
                return std::string("NE_MSG_ERROR");
            case NE_MSG_LOGIN:
                return std::string("NE_MSG_LOGIN");
            case NE_MSG_LOGIN_ACK:
                return std::string("NE_MSG_LOGIN_ACK");
            case NE_MSG_DISCONNECT:
                return std::string("NE_MSG_DISCONNECT");
            case NE_MSG_DISCONNECT_ACK:
                return std::string("NE_MSG_DISCONNECT_ACK");
            case NE_MSG_LOGIN_FAILED:
                return std::string("NE_MSG_LOGIN_FAILED");
            case NE_MSG_ENTRY_SET_START:
                return std::string("NE_MSG_ENTRY_SET_START");
            case NE_MSG_ENTRY:
                return std::string("NE_MSG_ENTRY");
            case NE_MSG_ENTRY_SET_END:
                return std::string("NE_MSG_ENTRY_SET_END");
            case NE_MSG_PING:
                return std::string("NE_MSG_PING");
            case NE_MSG_PONG:
                return std::string("NE_MSG_PONG");
            case NE_MSG_STATUS:
                return std::string("NE_MSG_STATUS");
            case NE_MSG_FORCED_DISCONNECT:
                return std::string("NE_MSG_FORCED_DISCONNECT");
            case NE_MSG_SEARCH_QUERY:
                return std::string("NE_MSG_SEARCH_QUERY");
            case NE_MSG_SEARCH_KEYWORD:
                return std::string("NE_MSG_SEARCH_KEYWORD");
            case NE_MSG_SEARCH_RESULTS:
                return std::string("NE_MSG_SEARCH_RESULTS");
            case NE_MSG_QUERY_RESULT:
                return std::string("NE_MSG_QUERY_RESULT");
            case NE_MSG_PUSH_REQUEST:
                return std::string("NE_MSG_PUSH_REQUEST");
            default:
                ret = "Unknown (FIXME: neUtils::msgNumToStr)";
        }
        return ret;
    }

    std::string fileToStr(char *filename)
    {
        FILE *fd = (FILE *)0;
        char buf[NE_MAX_BUF_READ_LEN] = {0};
        std::string outstr;

        if (filename && (fd = fopen(filename,"r")))
        {
            while(fread(buf,sizeof(char),NE_MAX_BUF_READ_LEN,fd))
            {
                outstr += buf;
                memset(buf,0,NE_MAX_BUF_READ_LEN);
            }
            fclose(fd);
        }
        return outstr;
    }

    std::string capitalizeStr(char *str)
    {
        std::string ret;
        int len = 0;
        char *ptr = (char *)0;
        char *ptrEnd = (char *)0;

        if (str)
        {
            len = strlen(str);
            ptr = str;
            ptrEnd = str + len;

            while(ptr < ptrEnd)
            {
                ret += (char)toupper((int)(*ptr++));
            }
        }
        return ret;
    }

    void zeroFile(int fd, unsigned long position, unsigned long bytes)
    {
        char buf[4096] = {0};
        unsigned long bytesRemaining = bytes;
        unsigned long bytesToWrite = MIN(bytes,4096);;

        if (fd && (fd != -1))
        {
            if (lseek(fd,position,SEEK_SET) != -1)
            {
                while(bytesRemaining)
                {
                    bytesRemaining -= write(fd,buf,bytesToWrite);
                    bytesToWrite = MIN(bytesRemaining,4096);
                }
            }
            else
            {
                eprintf("neUtils::zeroFile | cannot seek to position "
                        "%lu\n",position);
            }
        }
    }
}
