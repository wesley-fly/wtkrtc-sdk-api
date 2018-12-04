/*
 * Porting third party mempool functions by xiaofan.lee@wattertek.com
 */

#ifndef _memory_pool_h_
#define _memory_pool_h_

#include <assert.h>
#include <stdint.h>


#if _WIN32
#include "memory_pool_win.h"
#else
#include "memory_pool_posix.h"
#endif

namespace webrtc {

template<class MemoryType>
class MemoryPool
{
public:
    // Factory method, constructor disabled.
    static int32_t CreateMemoryPool(MemoryPool*& memoryPool,
                                    uint32_t initialPoolSize);

    // Try to delete the memory pool. Fail with return value -1 if there is
    // outstanding memory.
    static int32_t DeleteMemoryPool(
        MemoryPool*& memoryPool);

    // Get/return unused memory.
    int32_t PopMemory(MemoryType*&  memory);
    int32_t PushMemory(MemoryType*& memory);
private:
    MemoryPool(int32_t initialPoolSize);
    ~MemoryPool();

    MemoryPoolImpl<MemoryType>* _ptrImpl;
};

template<class MemoryType>
MemoryPool<MemoryType>::MemoryPool(int32_t initialPoolSize)
{
    _ptrImpl = new MemoryPoolImpl<MemoryType>(initialPoolSize);
}

template<class MemoryType>
MemoryPool<MemoryType>::~MemoryPool()
{
    delete _ptrImpl;
}

template<class MemoryType> int32_t
MemoryPool<MemoryType>::CreateMemoryPool(MemoryPool*&   memoryPool,
                                         uint32_t initialPoolSize)
{
    memoryPool = new MemoryPool(initialPoolSize);
    if(memoryPool == NULL)
    {
        return -1;
    }
    if(memoryPool->_ptrImpl == NULL)
    {
        delete memoryPool;
        memoryPool = NULL;
        return -1;
    }
    if(!memoryPool->_ptrImpl->Initialize())
    {
        delete memoryPool;
        memoryPool = NULL;
        return -1;
    }
    return 0;
}

template<class MemoryType>
int32_t MemoryPool<MemoryType>::DeleteMemoryPool(MemoryPool*& memoryPool)
{
    if(memoryPool == NULL)
    {
        return -1;
    }
    if(memoryPool->_ptrImpl == NULL)
    {
        return -1;
    }
    if(memoryPool->_ptrImpl->Terminate() == -1)
    {
        return -1;
    }
    delete memoryPool;
    memoryPool = NULL;
    return 0;
}

template<class MemoryType>
int32_t MemoryPool<MemoryType>::PopMemory(MemoryType*& memory)
{
    return _ptrImpl->PopMemory(memory);
}

template<class MemoryType>
int32_t MemoryPool<MemoryType>::PushMemory(MemoryType*& memory)
{
    if(memory == NULL)
    {
        return -1;
    }
    return _ptrImpl->PushMemory(memory);
}
}  // namespace webrtc

#endif
