#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include <cstddef>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder
{
    class ThreadCache : public NonCopyable, public NonMoveable
    {
    public:
        struct Block;
        struct BlockHeader
        {
            std::size_t idx;
            Block* next;
        };
        struct Block
        {
            BlockHeader header;
            void* data;
        };
        struct ListItem
        {
            std::size_t freeNums;
            std::size_t usedNums;
            std::size_t blockSize;
            std::size_t itemIdx;
            Block* freeList;
            Block* usedList;
            std::vector<Block*> rawPtrs; // Store raw pointers to blocks for deallocation
        };

        constexpr static std::size_t HEADER_SIZE = sizeof(BlockHeader);
        constexpr static std::size_t BLOCK_SIZE = sizeof(Block);
        constexpr static std::size_t MAX_SIZE = 256 * 1024; // 256KB
        constexpr static std::size_t MAX_FREE_BLOCK_NUMS = 1024;
        constexpr static std::size_t INIT_BLOCK_NUMS = 10;
        inline static bool MOOK_ALLOCATE = false; // For testing purposes, set to true to use mock allocation

        ThreadCache();
        ~ThreadCache();

        void* allocate(std::size_t size);
        void deallocate(void* ptr);

        static thread_local ThreadCache& getInstance()
        {
            static thread_local ThreadCache instance;
            return instance;
        }
    private:
        std::size_t index(std::size_t size) const;
        ListItem& getListItem(Block* block);
        std::size_t getAlignByIndex(std::size_t idx) const;

        Block* allocateBlock(ListItem& item);
        void deallocateBlock(Block* block);

        void fetchFromCentralCache(std::size_t blockSize, std::size_t blockNums = INIT_BLOCK_NUMS);
        void returnToCentralCache(ListItem& item);
    private:
        ListItem m_freeList[208];
    };


    class CentralCache : public NonCopyable, public NonMoveable
    {
    public:
        CentralCache() = default;
        ~CentralCache() = default;

        void* allocate(std::size_t size);
        void deallocate(void* ptr);

        static CentralCache& getInstance()
        {
            static CentralCache instance;
            return instance;
        }
    private:
        std::mutex m_mutex;
    };


    class PageCache : public NonCopyable, public NonMoveable
    {
    public:
        constexpr static std::size_t PAGE_SIZE = 4096; // 4KB
        constexpr static std::size_t SPAN_SIZE = 128 * PAGE_SIZE; // 512KB

        struct Span
        {
            Span* prev;
            Span* next;
        };

        PageCache();
        ~PageCache();

        void* fetchPages(std::size_t npages);
        void returnPages(void* span);

        static PageCache& getInstance()
        {
            static PageCache instance;
            return instance;
        }
    private:
        void allocateSpan();
        Span* splitSpan(std::size_t index, std::size_t npages);
    private:
        std::mutex m_mutex;
        Span* m_spanList[128];
        std::vector<void*> m_rawPtrs; // Store raw pointers to spans for deallocation
        std::unordered_map<Span*, std::size_t> m_spanMap;
    };
}
#endif // _MEMORY_POOL_H_