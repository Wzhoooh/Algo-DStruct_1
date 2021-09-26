#ifndef DYNAMIC_ARRAY_HPP_INCLUDED
#define DYNAMIC_ARRAY_HPP_INCLUDED

#include <memory>
#include <stdexcept>
#include <string>
#include <cmath>
#include <initializer_list>
#include <cstddef>


// https://en.cppreference.com/w/cpp/container/vector
template < typename T, typename Allocator = std::allocator<T> >
class DynArr
{
public:
    // member types
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename std::allocator_traits<Allocator>::pointer pointer;
    typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;

private:
    template < typename Alloc >
    using traits = std::allocator_traits<Alloc>;

public:
    // member functions
    DynArr() noexcept(noexcept(Allocator())):
            _p(nullptr), _size(0), _cap(0), _alloc(Allocator())
    {}

    explicit DynArr(const Allocator& alloc) noexcept: 
            _p(nullptr), _size(0), _cap(0), 
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {}

    DynArr(size_type count, const T& value, const Allocator& alloc = Allocator()):
            DynArr(alloc)
    {
        _cap = calcCapacity(count);
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (size_type i = 0; i < count; i++)
            push_back(value);
        
        _size = count;
    }

    explicit DynArr(size_type count, const Allocator& alloc = Allocator()):
            DynArr(alloc)
    {
        _cap = calcCapacity(count);
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (size_type i = 0; i < count; i++)
            push_back(value_type());

        _size = count;
    }

    DynArr(const DynArr& other): DynArr(other.get_allocator())
    {
        _cap = calcCapacity(other.size());
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (size_type i = 0; i < other.size(); i++)
            push_back(other.at(i));
    }

    DynArr(const DynArr& other, const Allocator& alloc): DynArr(alloc)
    {
        _cap = calcCapacity(other.size());
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (size_type i = 0; i < other.size(); i++)
            push_back(other.at(i));

        other.clear();
    }

    DynArr(DynArr&& other) noexcept: DynArr(other.get_allocator())
    {
        _cap = calcCapacity(other.size());
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (size_type i = 0; i < other.size(); i++)
            push_back(std::move_if_noexcept(other.at(i)));

        other.clear();
    }
    
    DynArr(DynArr&& other, const Allocator& alloc): DynArr(alloc)
    {
        _cap = calcCapacity(other.size());
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (size_type i = 0; i < other.size(); i++)
            push_back(std::move_if_noexcept(other.at(i)));

        other.clear();
    }
    
    DynArr(std::initializer_list<T> init, const Allocator& alloc = Allocator()): 
            DynArr(alloc)
    {
        _cap = calcCapacity(init.size());
        _p = traits<Allocator>::allocate(_alloc, sizeof(value_type) * capacity());
        for (auto i = init.begin(); i != init.end(); i++)
            push_back(*i);
    }

    ~DynArr()
    {
        clear();
    }

    // element access
    reference at(size_type pos)
    {
        if (!(pos < size()))
            throw std::out_of_range(std::string("DynArr::at: pos >= this.size() (which is ") + 
                    std::to_string(size()) + ")");
        
        return _p[pos];
    }

    const_reference at(size_type pos) const
    {
        if (!(pos < size()))
            throw std::out_of_range(std::string("DynArr::at: pos >= this.size() (which is ") + 
                    std::to_string(size()) + ")");
        
        return _p[pos];
    }

    reference operator[](size_type pos)
    {
        return _p[pos];
    }
    
    const_reference operator[](size_type pos) const
    {
        return _p[pos];
    }

    allocator_type get_allocator() const noexcept
    {
        _alloc;
    }

    // iterators
    // ...

    // capasity
    bool empty() const noexcept
    {
        return size() == 0;
    }
    
    size_type size() const noexcept
    {
        return _size;
    }

    size_type capacity() const noexcept
    {
        return _cap;
    }
    
    void shrink_to_fit()
    {
        auto newCap = size();
        auto newP = traits<Allocator>::allocate(_alloc, sizeof(value_type) * newCap);
        for (size_type i = 0; i < size(); i++)
            traits<Allocator>::construct(_alloc, newP + i, std::move_if_noexcept(_p[i]));

        traits<Allocator>::deallocate(_alloc, _p, capacity());
        _p = newP;
        _cap = newCap;
    }

    // modifiers
    void clear() noexcept
    {
        try
        {
            resize(0);
        }
        catch(...)
        {}
        traits<Allocator>::deallocate(_alloc, _p, capacity());
        _cap = 0;
    }

    void push_back(const T& value)
    {
        if (size() < capacity())
        {
            traits<Allocator>::construct(_alloc, _p + size(), value);
            _size++;
        }
        else
        {
            auto newCap = calcCapacity(size() + 1);
            auto newP = traits<Allocator>::allocate(_alloc, sizeof(value_type) * newCap);
            try
            {
                traits<Allocator>::construct(_alloc, newP + size(), value);
            }
            catch(...)
            {
                traits<Allocator>::deallocate(_alloc, newP, newCap);
                throw;
            }
            
            for (size_type i = 0; i < size(); i++)
                traits<Allocator>::construct(_alloc, newP + i, std::move_if_noexcept(_p[i]));

            traits<Allocator>::deallocate(_alloc, _p, capacity());
            _p = newP;
            _size++;
            _cap = newCap;
        }
    }
    
    void push_back(T&& value)
    {
        if (size() < capacity())
        {
            traits<Allocator>::construct(_alloc, _p + size(), value);
            _size++;
        }
        else
        {
            auto newCap = calcCapacity(size() + 1);
            auto newP = traits<Allocator>::allocate(_alloc, sizeof(value_type) * newCap);
            try
            {
                traits<Allocator>::construct(_alloc, newP + size(), std::move(value));
            }
            catch(...)
            {
                traits<Allocator>::deallocate(_alloc, newP, newCap);
                throw;
            }
            
            for (size_type i = 0; i < size(); i++)
                traits<Allocator>::construct(_alloc, newP + i, std::move_if_noexcept(_p[i]));

            traits<Allocator>::deallocate(_alloc, _p, capacity());
            _p = newP;
            _size++;
            _cap = calcCapacity(size());
        }
    }
    
    template <typename... Args >
        reference emplace_back(Args&&... args)
        {
            push_back(value_type(std::forward<Args...>(args...)));
            return at(size()-1);
        }
    
    void pop_back()
    {
        resize(size() - 1);
    }
    
    void resize(size_type count)
    {
        if (!(count <= size()))
            throw std::out_of_range(std::string("DynArr::resize: count > this.size() (which is ") + 
                    std::to_string(size()) + ")");

        for (size_type i = count; i < size(); i++)
            traits<Allocator>::destroy(_alloc, _p + i);
    
        _size = count;
    }

    void resize(size_type count, const T& val)
    {
        if (count < size())
        {
            for (size_type i = count; i < size(); i++)
                traits<Allocator>::destroy(_alloc, _p + i);
        
            _size = count;
        }
        else if (count > size())
            for (; size() < count;)
                push_back(val);
    }

private:
    size_type calcCapacity(size_type sz) const noexcept
    {
        // return std::pow(2, (size_type)std::log2(sz) + 1);
        return sz * 2;
    }

private:
    // fields
    pointer _p;
    size_type _size;
    size_type _cap;
    Allocator _alloc;
};

// non-member functions
template < typename T, typename Alloc >
std::ostream& operator<<(std::ostream& os, const DynArr<T, Alloc>& arr)
{
    if (arr.size() == 0)
    {
        std::cout << "[]";
        return os;
    }
    os << "[";
    for (int i = 0; i < arr.size()-1; i++)
        os << arr.at(i) << ", ";
    os << arr.at(arr.size()-1);
    os << "]";
    return os;
}


#endif // DYNAMIC_ARRAY_HPP_INCLUDED
