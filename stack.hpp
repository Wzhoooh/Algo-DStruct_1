#ifndef STACK_HPP_INCLUDED
#define STACK_HPP_INCLUDED

#include <memory>
#include <cstddef>
#include <iterator>
#include "forward_list.hpp"


// https://en.cppreference.com/w/cpp/container/stack
template < typename T, typename Container = ForwardList<T> >
class Stack
{
public:
    // member types
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;

public:
    // member functions
    Stack(): _size(0)
    {}
    Stack(const Stack& other): _cont(other._cont), _size(other._size)
    {}
    Stack(Stack&& other): _cont(std::move(other._cont)), _size(other._size)
    {}

    ~Stack()
    {}

    // element access
    reference top()
    {
        return _cont.front();
    }
    const_reference top() const
    {
        return _cont.front();
    }

    // capacity
    bool empty() const
    {
        return _cont.empty();
    }
    size_type size() const
    {
        return _size;
    }

    // modifiers
    void push(const value_type& value)
    {
        _cont.push_front(value);
        _size++;
    }

    void push(value_type&& value)
    {
        _cont.push_front(std::move(value));
        _size++;
    }

    template< class... Args >
    void emplace(Args&&... args)
    {
        return push(value_type(std::forward<Args...>(args...)));
    }

    void pop()
    {
        _cont.pop_front();
        _size--;
    }

private:
    Container _cont;
    size_type _size;
};


#endif // STACK_HPP_INCLUDED