#pragma once

#include <new>

template
<
    typename T
>
T* New(size_t size = 1)
{
    if (size)
        return new (std::nothrow) T[size];
    return nullptr;
}

template
<
    typename T
>
void Delete(T* ptr)
{
    delete[] ptr;
}

template
<
    typename T
>
class TRingQueue
{

    struct Item
    {
        T data;
        Item* next;
    };

    Item* m_item;
    Item* m_head;

public:

    TRingQueue()
        : m_head(nullptr)
        , m_item(nullptr)
    {
    }

    ~TRingQueue()
    {
        Delete(m_head);
    }

    bool Init(size_t size) 
    { 
        if (!m_head)
        {
            m_item = m_head = New<Item>(size);
            for (size_t i = 0; m_head && i < size; i++)
            {
                size_t n = i + 1;
                m_head[i].next = (n == size) ? &m_head[0] : &m_head[n];
            }
            return m_item != nullptr;
        }
        return false;
    }

    void Next() { m_item = m_item ? m_item->next : nullptr; }

    const T* Get() const { return m_item ? &m_item->data : nullptr; }

    T* Get() { return m_item ? &m_item->data : nullptr; }

    void Reset() { m_item = m_head; }

};
