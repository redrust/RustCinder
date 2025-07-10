#ifndef _MACRO_H_
#define _MACRO_H_

#include <cstddef>
#include <cstdlib>
#include <memory>

#include "common/memory_pool.h"

namespace RustCinder
{
    template <typename T, typename... Args>
    T* hookNew(Args... args)
    {
        void* ptr = ThreadCache::getInstance().allocate(sizeof(T));
        T* objPtr = new (ptr) T(std::forward<Args>(args)...);
        return objPtr;
    }

    template <typename T>
    void hookDelete(T* ptr) noexcept
    {
        if (!ptr)
        {
            std::cout << "Custom hookDelete called for null pointer." << std::endl;
            return;
        }
        ptr->~T(); // Call the destructor
        ThreadCache::getInstance().deallocate(ptr);
    }

    template <typename T, std::size_t N>
    T* hookNewMulti()
    {
        void* originalPtr = ThreadCache::getInstance().allocate(sizeof(T) * N + 4);
        std::size_t* header = reinterpret_cast<size_t*>(originalPtr);
        *header = N;
        void* ptr = reinterpret_cast<void*>(reinterpret_cast<char*>(originalPtr) + 4);
        T* objPtr = new (ptr) T[N]; // Placement new for array
        return objPtr;
    }

    template <typename T>
    void hookDeleteMulti(T* ptr) noexcept
    {
        if (!ptr)
        {
            std::cout << "Custom hookDeleteMulti called for null pointer." << std::endl;
            return;
        }
        void* originalPtr = reinterpret_cast<void*>(reinterpret_cast<char*>(ptr) - 4);
        size_t N = *reinterpret_cast<size_t*>(originalPtr);
        for (std::size_t i = 0; i < N; ++i)
        {
            ptr[i].~T(); // Call the destructor for each element
        }
        ThreadCache::getInstance().deallocate(originalPtr);
    }

    #define RUST_NEW hookNew
    #define RUST_DELETE hookDelete
    #define RUST_NEW_M hookNewMulti
    #define RUST_DELETE_M hookDeleteMulti
}

// 这里不进行全局的 new/delete 重载，因为其他底层代码库也会使用new/delete，然后thread local变量生命周期其实要比第三方库的静态变量要短的。
// 于是就会出现thread local的内存池已经析构了，但是还有内存没回收。
// 或者说非法访问了已经释放的内存，导致程序崩溃。
// 所以上面定义了几个宏来使用定制的内存池，仅限于项目内部自己使用，可以避免很多麻烦。
// 以及内存池内部本身使用了部分std标准库的内容，如果需要完全替代，需要手动实现部分标准库的容器。
// 综合考虑之后，项目内部自定义结构才使用内存池，第三方库的内容还是使用全局的new/delete。
// 项目内部也可以使用全局的new/delete，提供多种方案进行使用。
// void* operator new(std::size_t size)
// {
//     // std::cout << "Custom global new called for size: " << size << std::endl;
//     void* ptr = RustCinder::ThreadCache::getInstance().allocate(size);
//     std::cout << "Custom global new called at address: " << ptr << std::endl;
//     return ptr;
// }

// void operator delete(void* ptr) noexcept
// {
//     // std::cout << "Custom global delete called for pointer at address: " << ptr << std::endl;
//     if (!ptr)
//     {
//         std::cout << "Custom global delete called for null pointer." << std::endl;
//         return;
//     }
//     RustCinder::ThreadCache::getInstance().deallocate(ptr);
// }

#endif // _MACRO_H_
