#include <algorithm>
#include <cmath>
#include <iostream>

#include "common/memory_pool.h"

namespace RustCinder
{
    static std::size_t index(std::size_t size)
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

    static std::size_t getAlignByIndex(std::size_t idx)
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
            return ThreadCache::MAX_SIZE; // Default to max size for larger allocations
    }

    static std::size_t limitPages(std::size_t npages)
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
        if(blockSize == 0 || blockSize > MAX_SIZE)
        {
            return; // Invalid size, do not fetch
        }
        std::size_t totalSize = (blockSize + HEADER_SIZE) * blockNums;
        void* ptr = nullptr;
        if(!MOOK_ALLOCATE)
        {
            ptr = CentralCache::getInstance().allocate(totalSize);
        }
        else
        {
            ptr = malloc(totalSize);
        }
        if(ptr == nullptr)
        {
            return;
        }
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

    void ThreadCache::returnToCentralCache(ListItem& item)
    {
        if(!MOOK_ALLOCATE)
        {
            for(Block* block : item.rawPtrs)
            {
                CentralCache::getInstance().deallocate(reinterpret_cast<void*>(block)); // Return the raw pointers to the central cache
            }
            item.rawPtrs.clear(); // Clear the raw pointers after deallocation
        }
        else
        {
            for(Block* block : item.rawPtrs)
            {
                free(block); // Free the data pointer
            }
            item.rawPtrs.clear(); // Clear the raw pointers after deallocation
        }
        item.freeList = nullptr; // Clear the free list
        item.usedList = nullptr; // Clear the used list
        item.freeNums = 0; // Reset the number of free blocks
        item.usedNums = 0; // Reset the number of used blocks
    }

    ThreadCache::ListItem& ThreadCache::getListItem(Block* block)
    {
        std::size_t idx = block->header.idx;
        return m_freeList[idx];
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

    
    CentralCache::CentralCache()
    {
        std::size_t totalSize = 0;
        for(std::size_t i = 0; i < SPAN_LIST_SIZE; ++i)
        {
            SpanListItem& item = m_spanList[i];
            item.npages = INIT_SPAN_ITEM_NUMS; // Initialize the number of pages in the span
            totalSize += getAlignByIndex(i);
            item.spanSize = totalSize;
            // fetchFromPageCache(i, totalSize, INIT_SPAN_ITEM_NUMS); // Fetch the initial span from the page cache
        }
    }

    CentralCache::~CentralCache()
    {
        for(std::size_t i = 0; i < SPAN_LIST_SIZE; ++i)
        {
            SpanListItem& item = m_spanList[i];
            for(void* ptr : item.rawPtrs)
            {
                if (ptr)
                {
                    PageCache::getInstance().returnPages(ptr); // Return the raw pointers to the page cache
                }
            }
        }
    }

    void* CentralCache::allocate(std::size_t size)
    {
        std::size_t listIndex = index(size);
        if (listIndex >= SPAN_LIST_SIZE)
        {
            return nullptr; // Size exceeds the maximum supported size
        }
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
        SpanListItem& item = m_spanList[listIndex];
        if(item.freeList == nullptr)
        {
            fetchFromPageCache(listIndex, item.spanSize, INIT_SPAN_ITEM_NUMS);
        }
        Span* span = item.freeList;
        item.freeList = span->next; // Remove the span from the free list
        span->next = nullptr; // Reset the next pointer
        m_spanMap[span] = listIndex; // Store the span in the map with its size
        return reinterpret_cast<void*>(span); // Return the span pointer
    }

    void CentralCache::deallocate(void* ptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
        if(!ptr)
        {
            return; // No need to deallocate null pointers
        }
        Span* span = reinterpret_cast<Span*>(ptr);
        if(m_spanMap.find(span) == m_spanMap.end())
        {
            std::cerr << "Error: Attempt to deallocate a span that was not allocated by CentralCache." << std::endl;
            return; // Span was not allocated by this cache
        }

        std::size_t listIndex = m_spanMap[span];
        m_spanMap.erase(span); // Remove the span from the map

        SpanListItem& item = m_spanList[listIndex];
        Span* current = item.freeList;
        if (current == nullptr)
        {
            item.freeList = span; // If the free list is empty, set the first span
        }
        else
        {
            char* ptrChar = reinterpret_cast<char*>(ptr);
            Span* prev = nullptr;
            while(current != nullptr && 
                  reinterpret_cast<char*>(current) < ptrChar)
            {
                current = current->next; // Find the correct position to insert the span
                prev = current; // Keep track of the previous span
            }
            if (current != nullptr)
            {
                span->next = current; // Link the new span to the next span
            }
            else
            {
                span->next = nullptr; // Set the next pointer to nullptr if it's the last span
            }
            if(prev != nullptr)
            {
                prev->next = span; // Link the previous span to the new span
            }
        }
    }

    
    void CentralCache::fetchFromPageCache(std::size_t listIndex, std::size_t spanSize, std::size_t spanItemNums)
    {
        std::size_t npages = std::ceil(spanSize / static_cast<float>(PageCache::PAGE_SIZE)) * spanItemNums;
        npages = limitPages(npages); // Ensure npages is within limits
        void* ptr = PageCache::getInstance().fetchPages(npages);
        for(std::size_t i = 0; i < spanItemNums - 1; ++i)
        {
            Span* span = reinterpret_cast<Span*>(reinterpret_cast<char*>(ptr) + i * spanSize);
            span->next = span + 1;
        }
        Span* lastSpan = reinterpret_cast<Span*>(reinterpret_cast<char*>(ptr) + (spanItemNums - 1) * spanSize);
        lastSpan->next = nullptr; // Set the last span's next to nullptr

        SpanListItem& item = m_spanList[listIndex];
        item.freeList = reinterpret_cast<Span*>(ptr); // If the free list is empty, set the first span
        item.rawPtrs.push_back(ptr); // Store the raw pointer for deallocation
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

    void* PageCache::fetchPages(std::size_t npages)
    {
        std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex for thread safety
        npages = limitPages(npages);
        auto& item = m_spanList[npages - 1];
        Span* span = item.freeList;
        if (span != nullptr)
        {
            Span* next = span->next; // Get the next span in the list
            item.freeList = next; // Remove the span from the list
            if(next != nullptr)
            {
                next->prev = nullptr; // Reset the previous pointer of the next span
            }
            span->next = nullptr; // Reset the next pointer

            m_spanMap[span] = npages - 1;
            item.freePages -= 1; // Decrease the number of free pages
            return span;
        }
        else
        {
            for(std::size_t i = npages; i < 128; ++i)
            {
                if(m_spanList[i].freeList == nullptr)
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

        auto& item = m_spanList[npages - 1];
        span->next = item.freeList; // Add the span back to the list
        if (item.freeList != nullptr)
        {
            item.freeList->prev = span; // Set the previous pointer of the current first span
        }
        span->prev = nullptr; // Reset the previous pointer of the span
        item.freeList = span; // Add the span to the list for the corresponding number of pages
        item.freePages += 1; // Increase the number of free pages

        if(item.freePages >= MAX_FREE_PAGES)
        {
            mergeSpan(npages); // If the next span is contiguous, merge them
        }
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

        auto& item = m_spanList[127];
        item.npages = 128;
        item.freeList = span;
        item.freePages = 128; // Initialize the number of free pages
        item.usedList = nullptr; // Initialize the used list to nullptr

        m_rawPtrs.push_back(ptr); // Store the raw pointer for deallocation later
        return;
    }

    PageCache::Span* PageCache::splitSpan(std::size_t index, std::size_t npages)
    {
        auto& originItem = m_spanList[index];
        Span* span = originItem.freeList;
        originItem.freeList = span->next; // Remove the span from the list
        originItem.freePages -= 1;
        
        std::size_t remainingPages = index + 1 - npages; // Calculate remaining pages
        Span* remainSpan = reinterpret_cast<Span*>(reinterpret_cast<char*>(span) + remainingPages * PAGE_SIZE);
        remainSpan->prev = nullptr; // Reset the previous pointer of the remaining span
        remainSpan->next = nullptr; // Reset the next pointer of the remaining span
        auto& remainItem = m_spanList[remainingPages - 1];
        if(remainItem.freeList == nullptr)
        {
            remainItem.freeList = remainSpan; // Add the remaining span to the list
        }
        else
        {
            Span* lastSpan = remainItem.freeList;
            remainSpan->next = lastSpan;
            lastSpan->prev = remainSpan; // Link the new span to the last span
            remainItem.freeList = remainSpan; // Update the list with the new span
        }
        remainItem.freePages += 1;

        m_spanMap[span] = npages; // Store the span in the map
        return span;
    }

    void PageCache::mergeSpan(std::size_t index)
    {
        auto& item = m_spanList[index - 1];
        Span* current = item.freeList;
        std::vector<Span*> spansToMerge;
        spansToMerge.reserve(item.freePages); // Reserve space for spans to merge
        while(current != nullptr)
        {
            spansToMerge.push_back(current); // Collect all spans in the free list
            current = current->next; // Iterate through the free list
        }
        if(spansToMerge.empty())
        {
            return;
        }
        std::sort(spansToMerge.begin(), spansToMerge.end(), 
            [](Span* a, Span* b) { return reinterpret_cast<char*>(a) < reinterpret_cast<char*>(b); });
        std::vector<Span*> unmergedSpans;
        unmergedSpans.reserve(spansToMerge.size()); // Reserve space for unmerged spans
        for(std::size_t i = 0; i < spansToMerge.size() - 1;)
        {
            Span* currentSpan = spansToMerge[i];
            std::size_t jumpCount = 1;
            while (i + jumpCount <= spansToMerge.size() - 1 &&
                   reinterpret_cast<char*>(currentSpan) + PAGE_SIZE == reinterpret_cast<char*>(spansToMerge[i + jumpCount]))
            {
                jumpCount++;
            }
            if(jumpCount > 1)
            {
                Span* tailSpan = reinterpret_cast<Span*>(reinterpret_cast<char*>(currentSpan) + jumpCount * PAGE_SIZE);
                Span* t = currentSpan;
                for(std::size_t j = 0; j < jumpCount - 1; ++j)
                {
                    t->next = reinterpret_cast<Span*>(reinterpret_cast<char*>(t) + PAGE_SIZE);
                    t->next->prev = t; // Link the next span
                    t = t->next; // Move to the next span
                }
                t->next = nullptr; // Set the last span's next to nullptr
                std::size_t newIndex = limitPages(index * jumpCount);
                auto& nextItem = m_spanList[newIndex];
                if(nextItem.freeList == nullptr)
                {
                    nextItem.freeList = currentSpan; // If the next item is empty, set the current span as the free list
                    nextItem.freePages = jumpCount; // Set the number of free pages
                }
                else
                {
                    tailSpan->next = nextItem.freeList; // Link the tail span to the existing free list
                    nextItem.freeList->prev = tailSpan;
                    nextItem.freeList = currentSpan; // Update the free list with the new head
                    nextItem.freePages += jumpCount; // Increase the number of free pages
                }
            }
            else
            {
                unmergedSpans.push_back(currentSpan); // If not contiguous, keep the current span
            }
            i += jumpCount; // Adjust the index to skip the merged spans
        }
        if(unmergedSpans.empty())
        {
            return; // No spans to merge
        }
        for(std::size_t i = 0; i < unmergedSpans.size() - 1; ++i)
        {
            Span* newHead = unmergedSpans[i];
            Span* nextSpan = unmergedSpans[i + 1];
            nextSpan->next = nullptr;
            newHead->next = nextSpan;
            nextSpan->prev = newHead; // Link the new span to the previous one
        }
        item.freeList = unmergedSpans[0]; // Update the free list with the new head
        item.freePages = unmergedSpans.size(); // Update the number of free pages
    }
}