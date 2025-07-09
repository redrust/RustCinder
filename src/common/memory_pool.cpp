#include "common/memory_pool.h"
#include <iostream>
namespace RustCinder
{
    ThreadCache::ThreadCache()
    {
        std::size_t totalSize = 0;
        for(std::size_t i = 0; i < 208; ++i)
        {
            ListItem& item = m_freeList[i];
            item.freeNums = INIT_BLOCK_NUMS;
            item.usedNums = 0;
            item.itemIdx = i;
            item.freeList = nullptr;
            item.usedList = nullptr;
            totalSize += getAlignByIndex(i);
            item.blockSize = totalSize;
            // fetchFromCentralCache(totalSize);
        }
    }
    
    ThreadCache::~ThreadCache()
    {
        for(std::size_t i = 0; i < 208; ++i)
        {
            ListItem& item = m_freeList[i];
            returnToCentralCache(item); // Return all blocks to the central cache
        }
    }


    void* ThreadCache::allocate(std::size_t size)
    {
        if(size == 0 || size > MAX_SIZE)
        {
            return malloc(size); // Use system allocator for invalid sizes
        }
        std::size_t idx = index(size);
        ListItem& item = m_freeList[idx];
        Block* block = nullptr;
        if(item.freeList != nullptr)
        {
            block = allocateBlock(item);
        }
        if(item.freeList == nullptr)
        {
            // If the free list is empty, we can fetch from the central cache
            fetchFromCentralCache(item.blockSize);
            if(block == nullptr)
            {
                block = allocateBlock(item);
            }
        }
        return reinterpret_cast<void*>(reinterpret_cast<char*>(block) + HEADER_SIZE); // Return the data pointer
    }

    void ThreadCache::deallocate(void* ptr)
    {
        if(!ptr)
        {
            return; // No need to deallocate null pointers
        }
        Block* block = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) - HEADER_SIZE);
        deallocateBlock(block); // Deallocate the block

        ListItem& item = getListItem(block);
        if(item.freeNums >= MAX_FREE_BLOCK_NUMS && item.usedNums == 0)
        {
            returnToCentralCache(item); // If the free list is too large, return to central cache
        }
    }

    void ThreadCache::fetchFromCentralCache(std::size_t blockSize, std::size_t blockNums)
    {
        if(!MOOK_ALLOCATE)
        {

        }
        else
        {
            void* ptr = malloc((blockSize + HEADER_SIZE) * blockNums);
            std::size_t idx = index(blockSize);
            for(std::size_t i = 0; i < blockNums; ++i)
            {
                Block* block = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) + i * (blockSize + HEADER_SIZE));
                block->header.idx = idx;
                block->header.next = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) + (i + 1) * (blockSize + HEADER_SIZE));
            }
            Block* tail = reinterpret_cast<Block*>(reinterpret_cast<char*>(ptr) + (blockNums - 1) * (blockSize + HEADER_SIZE));
            tail->header.idx = idx;
            tail->header.next = nullptr; // Set the last block's next to nullptr
            
            ListItem& item = m_freeList[idx];
            item.freeList = reinterpret_cast<Block*>(ptr);
            item.freeNums += blockNums; // Increase the number of free blocks

            Block* newRaw = (Block*)malloc(BLOCK_SIZE);
            newRaw->data = ptr;
            if(item.rawPtr)
            {
                newRaw->header.next = item.rawPtr;
            }
            item.rawPtr = newRaw;
            return;
        }
    }

    void ThreadCache::returnToCentralCache(ListItem& item)
    {
        if(!MOOK_ALLOCATE)
        {

        }
        else
        {
            Block* current = item.rawPtr;
            Block* t = nullptr;
            while(current)
            {
                t = current;
                current = current->header.next; // Traverse the raw pointers
                free(t->data); // Free the data pointer
                free(t); // Free each raw pointer
            }

            item.freeList = nullptr; // Clear the free list
            item.usedList = nullptr; // Clear the used list
            item.freeNums = 0; // Reset the number of free blocks
            item.usedNums = 0; // Reset the number of used blocks
        }
    }

    std::size_t ThreadCache::index(std::size_t size) const
    {
        if(size <= 128)
        {
            return (size - 1) / 8; // 8-byte alignment
        }
        else if(size <= 1024)
        {
            return 16 + (size - 129) / 16; // 16-byte alignment
        }
        else if(size <= 8 * 1024)
        {
            return 72 + (size - 1025) / 128; // 128-byte alignment
        }
        else if(size <= 64 * 1024)
        {
            return 128 + (size - 8 * 1024) / 1024; // 256-byte alignment
        }
        else if(size <= 256 * 1024)
        {
            return 184 + (size - 64 * 1024) / (8 * 1024); // 1024-byte alignment
        }
        else
        {
            return -1;
        }
    }

    ThreadCache::ListItem& ThreadCache::getListItem(Block* block)
    {
        std::size_t idx = block->header.idx;
        return m_freeList[idx];
    }


    std::size_t ThreadCache::getAlignByIndex(std::size_t idx) const
    {
        if (idx < 16)
            return 8; // 8-byte alignment
        else if (idx < 72)
            return 16; // 16-byte alignment
        else if (idx < 128)
            return 128; // 128-byte alignment
        else if (idx < 184)
            return 1024; // 1024-byte alignment
        else if (idx < 208)
            return 8192; // 8192-byte alignment
        else
            return MAX_SIZE; // Default to max size for larger allocations
    }
    
    ThreadCache::Block* ThreadCache::allocateBlock(ListItem& item)
    {
        Block* block = item.freeList;

        item.freeList = block->header.next;

        block->header.next = item.usedList;
        item.usedList = block; // Move the block to the used list

        item.usedNums++;
        item.freeNums--; // Decrease the number of free blocks

        return block;
    }

    void ThreadCache::deallocateBlock(Block* block)
    {
        if(!block)
        {
            return;
        }
        ListItem& item = getListItem(block);
        block->header.next = item.freeList;
        item.freeList = block; // Add the block back to the free list

        Block* current = item.usedList;
        while(current && current->header.next != block)
        {
            current = current->header.next; // Find the block in the used list
        }
        if(current)
        {
            current->header.next = block->header.next; // Remove the block from the used list
        }

        item.usedNums--;
        item.freeNums++; // Increase the number of free blocks
    }
}