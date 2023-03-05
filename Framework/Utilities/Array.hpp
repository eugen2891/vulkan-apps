#pragma once

#include <cstdint>

#include "Debug.hpp"
#include "Memory.hpp"

template <typename T, uint32_t N> 
constexpr uint32_t CountOf(T(&)[N]) { return N; }

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
};
