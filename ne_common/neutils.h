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

#ifndef __NEUTILS_H
#define __NEUTILS_H

/* a set of utility functions wrapped in the "neUtils" namespace */
namespace neUtils
{
    unsigned long peekMessage(ncSocket *sock);
    char *getFormattedFilename(char *filename);
    std::string getFormattedOutputFilename(char *filename);
    char *getFormattedFilenameWithSubstitute(char *filename,
                                             char oldChar,
                                             char newChar);
    char *ipToStr(unsigned long ip);
    int computeMD5Checksum(char *filename, unsigned char digest[16]);
    int computeQuickMD5Checksum(char *filename, unsigned char digest[16]);
    char *getConnectionSpeedString(int connectionSpeed);
    int getConnectionSpeedValue(char *connectionSpeed);
    void discardSocketData(ncSocket *socket, int numBytes);
    void getFormattedTime(char *outstr);
    std::string msgNumToStr(unsigned long msgType);
    std::string fileToStr(char *filename);
    std::string capitalizeStr(char *str);
    void zeroFile(int fd, unsigned long position, unsigned long bytes);
}

#endif /* __NEUTILS_H */
