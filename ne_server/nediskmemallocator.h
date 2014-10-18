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

#ifndef __NEDISKMEMALLOCATOR_H
#define __NEDISKMEMALLOCATOR_H

/* a structure that associates an mmap'd fd to a start address */
typedef struct
{
    void *start;
    int fd;
} DMA_START;

class neDiskMemAllocator
{
  public:
    neDiskMemAllocator();

    ~neDiskMemAllocator();

    /* must be called before any allocations can work */
    int initialize(char *inputFile,
                   int objByteSize,
                   int initialCount);

    /*
      returns next free chunk.  this will grow the entire mmap'd
      region if required by a factor of m_growFactor;
      returns 0 on failure
    */
    void *alloc();

    /* returns a chunk to the free stack */
    void free(void *ptr);

    /* must be called when the allocator is no longer needed */
    void shutdown();

  private:
    int createDiskMemFile();

    int m_objByteSize;
    int m_count;
    int m_growFactor;
    int m_backingStoreCount;
    ncMutex m_mutex;
    std::vector< void * > m_usedVec;
    std::stack< void * > m_freeStack;
    std::string m_inputFileBase;
    std::map< std::string, DMA_START > m_fileToDSMap;
};

#endif /* __NEDISKMEMALLOCATOR_H */
