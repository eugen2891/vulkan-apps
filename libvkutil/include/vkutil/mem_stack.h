#pragma once

#include <stdint.h>

class MemStack
{

public:

    template <typename T>
    T* allocStack(uint32_t num) 
    { 
        if (!num)
            return nullptr;
        return static_cast<T*>(allocStack(sizeof(T) * num));
    }

    void* allocStack(size_t bytes);
    void* allocLinear(size_t bytes);
    void pushFrame();
    void popFrame();

protected:

    struct Marker
    {
        Marker* prev;
        size_t  offs;
    };

    char*   pMemBase = nullptr;
    Marker* pStFrame = nullptr;
    size_t  pmgBytes = 0;
    size_t  avlBytes = 0;

};

template <size_t N>
class TMemStack : public MemStack
{
public:
    TMemStack() { MemStack::pMemBase = memBlock; MemStack::avlBytes = N; }
private:
    char memBlock[N];
};
