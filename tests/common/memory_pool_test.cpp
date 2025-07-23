#include <iostream>

#include <gtest/gtest.h>

#include <pthread.h>
#include "common/memory_pool.h"
#include "common/macro.h"

TEST(MemoryPoolTest, AllocateDeallocate)
{
    RustCinder::ThreadCache::MOOK_ALLOCATE = true; // Enable mock allocation for testing
    class A
    {
    private:
        char data[100];
    };
    A* a = new A();
    delete a;
    A* a1 = RustCinder::RUST_NEW<A>();
    ASSERT_NE(a1, nullptr); // Check if allocation was successful
    RustCinder::RUST_DELETE(a1);

    A* arr = RustCinder::RUST_NEW_M<A, 10>();
    ASSERT_NE(arr, nullptr); // Check if array allocation was successful
    RustCinder::RUST_DELETE_M(arr);
}

TEST(MemoryPoolTest, PageCacheTest)
{
    void* ptr1 = RustCinder::PageCache::getInstance().fetchPages(1);
    void* ptr2 = RustCinder::PageCache::getInstance().fetchPages(2);
    void* ptr3 = RustCinder::PageCache::getInstance().fetchPages(125);
    void* ptr4 = RustCinder::PageCache::getInstance().fetchPages(1);
    RustCinder::PageCache::getInstance().returnPages(ptr1);
    ptr1 = RustCinder::PageCache::getInstance().fetchPages(1); // Re-fetch to check if it works after return
    ASSERT_NE(ptr1, nullptr); // Check if allocation was successful
    ASSERT_NE(ptr2, nullptr); // Check if allocation was successful
    ASSERT_NE(ptr3, nullptr); // Check if allocation was successful
    ASSERT_NE(ptr4, nullptr); // Check if allocation was successful
    RustCinder::PageCache::getInstance().returnPages(ptr1);
    RustCinder::PageCache::getInstance().returnPages(ptr2);
    RustCinder::PageCache::getInstance().returnPages(ptr3);
    RustCinder::PageCache::getInstance().returnPages(ptr4);
}

TEST(MemoryPoolTest, PageCacheMergeTest)
{
    std::vector<void*> spans;
    for(int i = 0; i < 256; ++i)
    {
        spans.push_back(RustCinder::PageCache::getInstance().fetchPages(2));
    }
    for(int i = 0; i < 256; ++i)
    {
        RustCinder::PageCache::getInstance().returnPages(spans[i]);
    }
}

TEST(MemoryPoolTest, CentralCacge)
{
    // Deallocate some pointers
    void* ptr1 = RustCinder::CentralCache::getInstance().allocate(1000);
    void* ptr2 = RustCinder::CentralCache::getInstance().allocate(2000);
    void* ptr3 = RustCinder::CentralCache::getInstance().allocate(3000);

    ASSERT_NE(ptr1, nullptr); // Check if allocation was successful
    ASSERT_NE(ptr2, nullptr); // Check if allocation was successful
    ASSERT_NE(ptr3, nullptr); // Check if allocation was successful

    RustCinder::CentralCache::getInstance().deallocate(ptr1);
    RustCinder::CentralCache::getInstance().deallocate(ptr2);
    RustCinder::CentralCache::getInstance().deallocate(ptr3);
}

TEST(MemoryPoolTest, NoMookAllocateDeallocate)
{
    class A
    {
    private:
        char data[100];
    };
    A* a = new A();
    delete a;
    A* a1 = RustCinder::RUST_NEW<A>();
    ASSERT_NE(a1, nullptr); // Check if allocation was successful
    RustCinder::RUST_DELETE(a1);

    A* arr = RustCinder::RUST_NEW_M<A, 10>();
    ASSERT_NE(arr, nullptr); // Check if array allocation was successful
    RustCinder::RUST_DELETE_M(arr);
}


int main(int argc, char** argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}