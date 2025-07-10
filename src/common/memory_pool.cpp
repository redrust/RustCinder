#include "common/memory_pool.h"

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
            fetchFromCentralCache(totalSize);
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

            item.rawPtrs.push_back(reinterpret_cast<Block*>(ptr)); // Store the raw pointer for deallocation
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
            for(Block* block : item.rawPtrs)
            {
                free(block); // Free the data pointer
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
        // else if (idx < 208)
        //     return 8192; // 8192-byte alignment
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


    PageCache::PageCache()
    {
        allocateSpan();
    }

    PageCache::~PageCache()
    {
        for(void* ptr : m_rawPtrs)
        {
            if (ptr)
            {
                free(ptr); // Free the raw pointers stored in m_rawPtrs
            }
        }
    }

    std::size_t limitPages(std::size_t npages)
    {
        if(npages <= 0)
        {
            npages = 1; // Ensure at least one page is fetched
        }
        else if (npages > 128)
        {
            npages = 128; // Limit to 128 pages
        }
        return npages;
    }

    void* PageCache::fetchPages(std::size_t npages)
    {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
        npages = limitPages(npages);
        Span* span = m_spanList[npages - 1];
        if (span != nullptr)
        {
            Span* next = span->next; // Get the next span in the list
            m_spanList[npages - 1] = next; // Remove the span from the list
            if(next != nullptr)
            {
                next->prev = nullptr; // Reset the previous pointer of the next span
            }
            span->next = nullptr; // Reset the next pointer

            m_spanMap[span] = npages - 1;
            return span;
        }
        else
        {
            for(std::size_t i = npages; i < 128; ++i)
            {
                if(m_spanList[i] == nullptr)
                {
                    continue;
                }
                else
                {
                    return splitSpan(i, npages);
                }
            }

            allocateSpan();
            return splitSpan(127, npages);
        }
        return nullptr;
    }
    
    void PageCache::returnPages(void* ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
        if(!ptr)
        {
            return;
        }
        Span* span = reinterpret_cast<Span*>(ptr);
        std::size_t npages = m_spanMap[span];
        npages = limitPages(npages); // Ensure npages is within limits
        m_spanMap.erase(span); // Remove the span from the map
        span->next = m_spanList[npages]; // Add the span back to the list
        if (m_spanList[npages] != nullptr)
        {
            m_spanList[npages]->prev = span; // Set the previous pointer of the current first span
        }
        span->prev = nullptr; // Reset the previous pointer of the span
        m_spanList[npages] = span; // Add the span to the list for the corresponding number of pages
    }
    
    void PageCache::allocateSpan()
    {
        void* ptr = malloc(SPAN_SIZE);
        if (!ptr)
        {
            throw std::bad_alloc();
        }
        Span* span = reinterpret_cast<Span*>(ptr);
        span->prev = nullptr;
        span->next = nullptr;
        m_spanList[127] = span;

        m_rawPtrs.push_back(ptr); // Store the raw pointer for deallocation later

        return;
    }

    PageCache::Span* PageCache::splitSpan(std::size_t index, std::size_t npages)
    {
        Span* span = m_spanList[index];
        m_spanList[index] = span->next; // Remove the span from the list
        
        std::size_t remainingPages = index + 1 - npages; // Calculate remaining pages
        Span* remainSpan = reinterpret_cast<Span*>(reinterpret_cast<char*>(span) + remainingPages * PAGE_SIZE);
        remainSpan->prev = nullptr; // Reset the previous pointer of the remaining span
        remainSpan->next = nullptr; // Reset the next pointer of the remaining span
        if(m_spanList[remainingPages - 1] == nullptr)
        {
            m_spanList[remainingPages - 1] = remainSpan; // Add the remaining span to the list
        }
        else
        {
            Span* lastSpan = m_spanList[remainingPages - 1];
            remainSpan->next = lastSpan;
            lastSpan->prev = remainSpan; // Link the new span to the last span
            m_spanList[remainingPages - 1] = remainSpan; // Update the list with the new span
        }

        m_spanMap[span] = npages - 1; // Store the span in the map
        return span;
    }
}