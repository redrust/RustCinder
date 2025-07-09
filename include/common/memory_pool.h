#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include <cstddef>
#include <mutex>
#include <vector>

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
            Block* rawPtr; // Store raw pointers for returning to central cache
        };

        constexpr static std::size_t HEADER_SIZE = sizeof(BlockHeader);
        constexpr static std::size_t BLOCK_SIZE = sizeof(Block);
        constexpr static std::size_t MAX_SIZE = 256 * 1024; // 256KB
        constexpr static std::size_t MAX_FREE_BLOCK_NUMS = 1024;
        constexpr static std::size_t INIT_BLOCK_NUMS = 10;
        inline static bool MOOK_ALLOCATE = true; // For testing purposes, set to true to use mock allocation

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
}
#endif // _MEMORY_POOL_H_