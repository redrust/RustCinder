#include <gtest/gtest.h>

#include "common/memory_pool.h"
#include "common/macro.h"

TEST(MemoryPoolTest, AllocateDeallocate)
{
    class A
    {
    private:
        char data[100];
    };
    // A* a = new A();
    // delete a;
    A* a = RustCinder::RUST_NEW<A>();
    ASSERT_NE(a, nullptr); // Check if allocation was successful
    RustCinder::RUST_DELETE(a);

    A* arr = RustCinder::RUST_NEW_M<A, 10>();
    ASSERT_NE(arr, nullptr); // Check if array allocation was successful
    RustCinder::RUST_DELETE_M(arr);
}


int main(int argc, char** argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}