#pragma once

#include "Debug.hpp"
#include "Memory.hpp"

template <typename T, uint32_t N> 
constexpr uint32_t CountOf(T(&)[N]) 
{ 
	return N; 
}

/*
template <typename T>
struct ArrayRef
{
	explicit ArrayRef() = default;
	ArrayRef(ArrayRef&&) = default;
	ArrayRef(const ArrayRef&) = delete;
	T* begin() const { return items; }
	T* end() const { return items + num; }
	ArrayRef(T* ptr, uint32_t cnt) : items(ptr), num(cnt) {}
	T& operator[](uint32_t i)
	{
		BreakIfNot(i < num);
		return items[i];
	}
	const T& operator[](uint32_t i) const
	{
		BreakIfNot(i < num);
		return items[i];
	}
	ArrayRef& operator=(ArrayRef&& rhs) noexcept
	{
		items = rhs.items;
		rhs.items = nullptr;
		num = rhs.num;
		rhs.num = 0;
		return *this;
	};
	T* items = nullptr;
	uint32_t num = 0;
};

template <typename T>
struct Array : public ArrayRef<T>
{
	explicit Array() = default;
	Array(Array&&) = default;
	Array(const Array&) = delete;
	explicit Array(T* ptr, uint32_t cnt)
	{
		this->items = ptr;
		this->num = cnt;
	}
	virtual ~Array()
	{
		delete[] this->items;
	}
	static Array<T> New(uint32_t count)
	{
		return Array<T>{ new T[count], count };
	}
	void clear() 
	{ 
		delete[] this->items; 
		this->items = nullptr;
		this->num = 0;
	}
	Array& operator=(Array&& rhs) noexcept
	{
		this->items = rhs.items;
		rhs.items = nullptr;
		this->num = rhs.num;
		rhs.num = 0;
		return *this;
	};
};*/

template <typename T>
class Range
{
public:

	Range() noexcept = default;

	Range(const Range&) = delete;

	Range(Range&& rhs) noexcept
	{
		m_items = rhs.m_items;
		rhs.m_items = nullptr;
		m_num = rhs.m_num;
		rhs.m_num = 0;
	}

	explicit Range(const std::vector<T>& vector) noexcept
		: m_items(vector.data())
		, m_num(vector.size())
	{
	}

	explicit Range(std::vector<T>& vector) noexcept
		: m_items(vector.data())
		, m_num(vector.size())
	{
	}

	constexpr Range(T* ptr, size_t num)  noexcept
		: m_items(ptr)
		, m_num(num)
	{
	}

	Range& operator=(Range&& rhs) noexcept
	{
		m_items = rhs.m_items;
		rhs.m_items = nullptr;
		m_num = rhs.m_num;
		rhs.m_num = 0;
		return *this;
	};

	T* begin() const noexcept
	{
		return m_items;
	}

	T* end() const noexcept
	{
		return m_items + m_num;
	}

	const T* get() const noexcept
	{
		return m_items;
	}

	T* get() noexcept
	{
		return m_items;
	}

	size_t num() const noexcept
	{
		return m_num;
	}

	void copy(Range& dst) noexcept
	{
		std::copy_n<T>(m_items, m_num, dst.m_items);
	}

	T& operator[](size_t i) noexcept
	{
		BreakIfNot(i < m_num);
		return m_items[i];
	}

	const T& operator[](size_t i) const noexcept
	{
		BreakIfNot(i < m_num);
		return m_items[i];
	}

	const T& operator*() const noexcept
	{
		return *m_items;
	}

	T& operator*() noexcept
	{
		return *m_items;
	}	

protected:
	T* m_items = nullptr;
	size_t m_num = 0;
};
