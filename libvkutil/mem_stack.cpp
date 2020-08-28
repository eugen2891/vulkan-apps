#include <vkutil/mem_stack.h>

#define ALIGN16(n) ((((n-1)>>4)+1)<<4)

void* MemStack::allocStack(size_t bytes)
{
    size_t size = ALIGN16(bytes);
    if (size <= avlBytes)
    {
        avlBytes -= size;
        return (pMemBase + pmgBytes + avlBytes);
    }
    return nullptr;
}

void* MemStack::allocLinear(size_t bytes)
{
    size_t size = ALIGN16(bytes);
    if (size <= avlBytes)
    {
        char* retVal = pMemBase + pmgBytes;
        pmgBytes += size;
        avlBytes -= size;
        return retVal;
    }
    return nullptr;
}

void MemStack::pushFrame()
{
    size_t offs = pmgBytes + avlBytes;
    Marker* nextFrame = static_cast<Marker*>(allocStack(sizeof(Marker)));
    nextFrame->prev = pStFrame;
    nextFrame->offs = offs;
    pStFrame = nextFrame;
}

void MemStack::popFrame()
{
    if (pStFrame)
    {
        Marker* frame = pStFrame->prev;
        avlBytes = pStFrame->offs - pmgBytes;
        pStFrame = frame;
    }
}
