#pragma once
#include <memory>
template<class T> class static_array //dynamically allocated, but cannot be resized like a vector
{
    T *vals;
    size_t sz;
public:
    static_array()
    {
        vals = nullptr;
        sz = 0;
    }
    static_array(size_t sz)
    {
        this->sz = sz;
        vals = new T[sz];
    }
    static_array(static_array &&other)
    {
        vals = other.vals;
        sz = other.sz;
        other.vals = nullptr;
        other.sz = 0;
    }
    static_array(const static_array &other)
    {
        sz = other.sz;
        vals = new T[sz];
        for(size_t i=0; i<sz; i++)
            vals[i] = other.vals[i];
    }
    static_array(std::initializer_list<T> x)
    {
        this->sz = x.size();
        vals = new T[sz];
        int pos = 0;
        for(auto &i: x)
            vals[pos++] = i;
    }
    static_array &operator = (static_array &&other)
    {
        if(vals != nullptr)
            delete[] vals;
        vals = other.vals;
        sz = other.sz;
        other.vals = nullptr;
        other.sz = 0;
        return *this;
    }
    static_array &operator = (const static_array &other)
    {
        if(vals != nullptr)
            delete[] vals;
        sz = other.sz;
        vals = new T[sz];
        for(size_t i=0; i<sz; i++)
            vals[i] = other.vals[i];
        return *this;
    }
    void clear()
    {
        if(vals != nullptr)
        {
            delete[] vals;
            vals = nullptr;
            sz = 0;
        }
    }
    [[gnu::always_inline]] T &operator[](size_t idx)
    {
        return vals[idx];
    }
    [[gnu::always_inline]] T get(size_t idx) const
    {
        return vals[idx];
    }
    [[gnu::always_inline]] size_t size() const
    {
        return sz;
    }
    [[gnu::always_inline]] T *begin()
    {
        return vals;
    }
    [[gnu::always_inline]] T *end()
    {
        return vals + sz;
    }
    ~static_array()
    {
        delete[] vals;
    }
};
