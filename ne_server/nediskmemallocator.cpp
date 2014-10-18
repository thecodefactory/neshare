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

#include "neserverheaders.h"

neDiskMemAllocator::neDiskMemAllocator()
{
    m_objByteSize = 0;
    m_count = 0;
    m_growFactor = 2;
    m_backingStoreCount = 0;
    assert(m_usedVec.empty());
    assert(m_freeStack.empty());
    assert(m_fileToDSMap.empty());
}

neDiskMemAllocator::~neDiskMemAllocator()
{
    m_mutex.lock();
    if (!m_usedVec.empty())
    {
        eprintf("neDiskMemAllocator::~neDiskMemAllocator | "
                "There are allocations still in use!\n"
                "Disk mem allocator was not shut down properly!\n");
    }
    m_mutex.unlock();
    m_objByteSize = 0;
    m_count = 0;
    m_growFactor = 0;
    m_backingStoreCount = 0;
}

int neDiskMemAllocator::initialize(char *inputFile,
                                   int objByteSize,
                                   int initialCount)
{

    m_objByteSize = objByteSize;
    m_count = initialCount;
    m_growFactor = 2;
    assert(m_usedVec.empty());
    assert(m_freeStack.empty());
    m_inputFileBase = std::string(inputFile);

    return createDiskMemFile();
}

int neDiskMemAllocator::createDiskMemFile()
{
    int ret = 1;
    int fd = 0;
    int chunkCount = 0;
    void *data = (void *)0;
    char *dataPtr = (char *)0;
    char *dataEnd = (char *)0;
    char buf[NE_MSG_MAX_DATA_LEN] = {0};

    m_mutex.lock();

    /* generate the next mem file name to create */
    snprintf(buf,NE_MSG_MAX_DATA_LEN,"%s%d",
             m_inputFileBase.c_str(),m_backingStoreCount);
    std::string inputFile = std::string(buf);
    m_backingStoreCount++;

    /* open or create the file */
    fd = open(inputFile.c_str(),(O_CREAT | O_RDWR),
              (S_IRUSR | S_IWUSR));
    if (fd != -1)
    {
        iprintf("createDiskMemFile | zeroing region of %lu bytes\n",
                (m_count * m_objByteSize));
        neUtils::zeroFile(fd,0,(m_count * m_objByteSize));

        /*
          be sure to rewind the fd before mmapping
          it after having it zeroed
        */
        lseek(fd,0,SEEK_SET);
        data = mmap(0,(m_count * m_objByteSize),
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_SHARED,fd,0);
        if (data != (void *)-1)
        {
            /* store the file information in our book keeping */
            DMA_START dms;
            memset(&dms,0,sizeof(DMA_START));
            dms.fd = fd;
            dms.start = data;
            m_fileToDSMap[inputFile] = dms;

            /*
              once we have a valid mmap'd file, break it up into
              chunks and place them all on the free stack
            */
            dataEnd = ((char *)data + (m_count * m_objByteSize));

            for(dataPtr = (char *)data;
                dataPtr != dataEnd;
                dataPtr += m_objByteSize)
            {
                m_freeStack.push(dataPtr);
                chunkCount++;
            }
            iprintf("Initialized backing store %s with %d chunks\n",
                    inputFile.c_str(),chunkCount);
            ret = 0;
        }
        else
        {
            eprintf("neDiskMemAllocator::createDiskMemFile | "
                    "Cannot mmap %s!\n",inputFile.c_str());
        }
    }
    else
    {
        eprintf("neDiskMemAllocator::createDiskMemFile | "
                "%s cannot be created or opened!\n",
                inputFile.c_str());
    }
    m_mutex.unlock();
    return ret;
}

void *neDiskMemAllocator::alloc()
{
    void *ptr = (void *)0;
    int stackEmpty = 0;

    m_mutex.lock();
    stackEmpty = (m_freeStack.empty() ? 1 : 0);
    m_mutex.unlock();

    if (stackEmpty && createDiskMemFile())
    {
        eprintf("neDiskMemAllocator::alloc | cannot allocate "
                "any more memory from the mmap'd regions!\n");
        return ptr;
    }

    m_mutex.lock();
    if (!m_freeStack.empty())
    {
        ptr = m_freeStack.top();
        m_freeStack.pop();

        m_usedVec.push_back(ptr);
    }
    m_mutex.unlock();
    return ptr;
}

void neDiskMemAllocator::free(void *ptr)
{
    std::vector<void *>::iterator iter;

    if (ptr)
    {
        m_mutex.lock();
        if ((iter = find(m_usedVec.begin(),m_usedVec.end(),ptr)) !=
            m_usedVec.end())
        {
            m_usedVec.erase(iter);
            m_freeStack.push(ptr);
        }
        else
        {
            /* remove this when we're sure this never happens */
            eprintf("neDiskMemAllocator::free | FIXME: Freeing "
                    "memory not allocated by me! (%x)\n",ptr);
        }
        m_mutex.unlock();
    }
    else
    {
        eprintf("neDiskMemAllocator::free | Error! Freeing NULL\n");
    }
}

void neDiskMemAllocator::shutdown()
{
    std::map< std::string, DMA_START >::iterator iter;

    m_mutex.lock();
    for(iter = m_fileToDSMap.begin();
        iter != m_fileToDSMap.end(); iter++)
    {
        iprintf("Deleting disk memory file %s\n",
                ((*iter).first).c_str());
        remove(((*iter).first).c_str());
        DMA_START &dms = (*iter).second;
        munmap(dms.start,(m_count * m_objByteSize));
        close(dms.fd);
    }
    m_fileToDSMap.clear();

    if (!m_usedVec.empty())
    {
        eprintf("neDiskMemAllocator::shutdown | "
                "There are allocations still in use!\n");
    }
    m_usedVec.clear();

    while(!m_freeStack.empty())
    {
        m_freeStack.pop();
    }
    m_mutex.unlock();
}
