#ifndef FORWARD_LIST_HPP_INCLUDED
#define FORWARD_LIST_HPP_INCLUDED

#include <memory>
#include <cstddef>
#include <iterator>


template < typename T >
struct Node
{
    typedef T value_type;
    typedef value_type* pointer;
    typedef Node<value_type> node_type;
    typedef node_type* node_pointer;

    Node(): value(nullptr), next(nullptr)
    {}
    Node(pointer value, node_pointer next):
        value(value), next(next)
    {}
    Node(const Node& other) = default;
    Node(Node&& other) = default;

    operator Node<const value_type>() const noexcept
    {
        return Node<const value_type>((const value_type*)value, next);
    }
    operator Node<std::remove_const<value_type>>() const noexcept
    {
        return Node<std::remove_const<value_type>>((std::remove_const<value_type>*)value, next);
    }

    pointer value;
    node_pointer next;
};

// https://en.cppreference.com/w/cpp/named_req/ForwardIterator
template < typename T >
class ForwardIterator
{
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

private:
    typedef typename Node<T>::node_type node_type;
    typedef typename Node<T>::node_pointer node_pointer;

template < typename T1, typename Allocator >
    friend class ForwardList;

template < typename T1 >
    friend class ForwardIterator;

private:
    node_pointer _getNodePointer() noexcept
    {
        return _node;
    }

    const node_pointer _getNodePointer() const noexcept
    {
        return _node;
    }

    ForwardIterator(node_pointer node): _node(node)
    {}

    operator ForwardIterator<std::remove_const<value_type>>() const noexcept
    {
        auto nodeP = (Node<std::remove_const<value_type>>*)_node;
        return ForwardIterator<std::remove_const<value_type>>(nodeP);
    }

public:
    ForwardIterator(): _node(nullptr)
    {}
    ForwardIterator(const ForwardIterator& other) = default;

    ~ForwardIterator()
    {}

    reference operator*()
    {
        return *(_node->value);
    }
    const_reference operator*() const
    {
        return *(_node->value);
    }

    pointer operator->()
    {
        return _node->value;
    }
    const_pointer operator->() const
    {
        return _node->value;
    }

    ForwardIterator& operator++()
    {
        _node = _node->next;
        return *this;
    }
    ForwardIterator& operator++(int)
    {
        ForwardIterator<value_type>oldIter(*this);
        _node = _node->next;
        return oldIter;
    }

    bool operator==(const ForwardIterator& other) const
    {
        return _node == other._node;
    }
    bool operator!=(const ForwardIterator& other) const
    {
        return !(*this == other);
    }

    operator ForwardIterator<const value_type>() const noexcept
    {
        auto nodeP = (Node<const value_type>*)_node;
        return ForwardIterator<const value_type>(nodeP);
    }

private:
    node_pointer _node;
};

// https://en.cppreference.com/w/cpp/container/forward_list
template < typename T, typename Allocator = std::allocator<T> >
class ForwardList
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
    typedef ForwardIterator<value_type> iterator;
    typedef ForwardIterator<const value_type> const_iterator;

private:
    template < typename Alloc >
    using traits = std::allocator_traits<Alloc>;

    typedef typename Allocator::template rebind<Node<T>>::other NodeAllocator;
    typedef Node<T> node_type;
    typedef node_type& node_reference;
    typedef const value_type& const_node_reference;
    typedef typename std::allocator_traits<NodeAllocator>::pointer node_pointer;
    typedef typename std::allocator_traits<NodeAllocator>::const_pointer const_node_pointer;

public:
    // member functions
    ForwardList()
    {}

    explicit ForwardList(const Allocator& alloc):
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {}

    ForwardList(size_type count, const T& value, const Allocator& alloc = Allocator()):
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {
        for (size_type i = 0; i < count; i++)
            push_front(value);
    }
    
    explicit ForwardList(size_type count, const Allocator& alloc = Allocator()):
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {
        for (size_type i = 0; i < count; i++)
            push_front(value_type());
    }
    
    ForwardList(const ForwardList& other)
    {
        for (auto iTh = before_begin(), iOt = other.begin(); iOt != other.end(); iTh++, iOt++)
            insert_after(iTh, *iOt);
    }
    
    ForwardList(const ForwardList& other, const Allocator& alloc):
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {
        for (auto iTh = before_begin(), iOt = other.begin(); iOt != other.end(); iTh++, iOt++)
            insert_after(iTh, *iOt);
    }
    
    ForwardList(ForwardList&& other)
    {
        _beforeBegin._getNodePointer()->next = other._beforeBegin._getNodePointer()->next;
        other._beforeBegin._getNodePointer()->next = nullptr;
    }
    
    ForwardList(ForwardList&& other, const Allocator& alloc):
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {
        _beforeBegin._getNodePointer()->next = other._beforeBegin._getNodePointer()->next;
        other._beforeBegin._getNodePointer()->next = nullptr;
    }
    
    ForwardList(std::initializer_list<T> init, const Allocator& alloc = Allocator()):
            _alloc(traits<Allocator>::select_on_container_copy_construction(_alloc))
    {
        for (auto iTh = before_begin(), iIn = init.begin(); iIn != init.end(); iTh++, iIn++)
            insert_after(iTh, *iIn);
    }

    ~ForwardList()
    {
        for (; !empty();)
            pop_front();

        traits<NodeAllocator>::deallocate(_nodeAlloc, _beforeBegin._node, sizeof(node_type));
        traits<NodeAllocator>::deallocate(_nodeAlloc, _end._node, sizeof(node_type));
    }

    allocator_type get_allocator() const noexcept
    {
        return _alloc;
    }

    // element access
    reference front()
    {
        return *begin();
    }
    const_reference front() const
    {
        return *begin();
    }

    // iterators
    iterator before_begin() noexcept
    {
        return _beforeBegin;
    }
    const_iterator before_begin() const noexcept
    {
        return (const_iterator)_beforeBegin;
    }
    const_iterator cbefore_begin() const noexcept
    {
        return (const_iterator)_beforeBegin;
    }

    iterator begin() noexcept
    {
        if (empty())
            return _end;
        else
        {
            auto it = _beforeBegin;
            return ++it;
        }
    }
    const_iterator begin() const noexcept
    {
        if (empty())
            return (const_iterator)_end;
        else
        {
            auto it = _beforeBegin;
            return (const_iterator)++it;
        }
    }
    const_iterator cbegin() const noexcept
    {
        if (empty())
            return (const_iterator)_end;
        else
        {
            auto it = _beforeBegin;
            return (const_iterator)++it;
        }
    }
    

    iterator end() noexcept
    {
        return _end;
    }
    const_iterator end() const noexcept
    {
        return (const_iterator)_end;
    }
    const_iterator cend() const noexcept
    {
        return (const_iterator)_end;
    }

    // capacity
    bool empty() const noexcept
    {
        return _beforeBegin._getNodePointer()->next == nullptr;
    }

    // modifiers
    iterator insert_after(const_iterator pos, const T& value)
    {
        // create object
        pointer obj = traits<Allocator>::allocate(_alloc, 1);
        traits<Allocator>::construct(_alloc, obj, value);
        // get pointer to next node
        auto nextNode = (node_pointer)pos._getNodePointer()->next;
        // create node
        node_pointer nodeP = traits<NodeAllocator>::allocate(_nodeAlloc, 1);
        traits<NodeAllocator>::construct(_nodeAlloc, nodeP, obj, nextNode);
        // change pointer to next node in previous node
        pos._getNodePointer()->next = (typename const_iterator::node_pointer)nodeP;
        return iterator(nodeP);
    }
    iterator insert_after(const_iterator pos, T&& value)
    {
        // create object
        pointer obj = traits<Allocator>::allocate(_alloc, 1);
        traits<Allocator>::construct(_alloc, obj, std::move(value));
        // get pointer to next node
        auto nextNode = (node_pointer)pos._getNodePointer()->next;
        // create node
        node_pointer nodeP = traits<NodeAllocator>::allocate(_nodeAlloc, 1);
        traits<NodeAllocator>::construct(_nodeAlloc, nodeP, obj, nextNode);
        // change pointer to next node in previous node
        pos._getNodePointer()->next = (typename const_iterator::node_pointer)nodeP;
        return iterator(nodeP);
    }

    template< class... Args >
    iterator emplace_after(const_iterator pos, Args&&... args)
    {
        return insert_after(pos, value_type(std::forward<Args...>(args...)));
    }

    iterator erase_after(const_iterator pos)
    {
        // get pointer to next node
        auto deletingNode = (node_pointer)pos._getNodePointer()->next;
        // get pointer to node after deleting
        auto nextNode = deletingNode->next;
        // delete node
        _deleteNode(deletingNode);
        // change pointer to next node in previous node
        pos._getNodePointer()->next = (typename const_iterator::node_pointer)nextNode;
        return iterator(nextNode);
    }
    
    void push_front(const T& value)
    {
        insert_after(_beforeBegin, value);
    }
    void push_front(T&& value)
    {
        insert_after(_beforeBegin, std::move(value));
    }

    template< class... Args >
    reference emplace_front(Args&&... args)
    {
        return emplace_after(_beforeBegin, value_type(std::forward<Args...>(args...)));
    }

    void pop_front()
    {
        auto i = before_begin();
        erase_after(i);
    }

private:
    void _deleteNode(node_pointer p)
    {
        traits<Allocator>::destroy(_alloc, p->value);
        traits<Allocator>::deallocate(_alloc, p->value, 1);
        traits<NodeAllocator>::destroy(_nodeAlloc, p);
        traits<NodeAllocator>::deallocate(_nodeAlloc, p, 1);
    }

private:
    Allocator _alloc;
    NodeAllocator _nodeAlloc;

    iterator _end = iterator(nullptr);
    iterator _beforeBegin = [&]() -> iterator {
        node_pointer p = traits<NodeAllocator>::allocate(_nodeAlloc, 1);
        traits<NodeAllocator>::construct(_nodeAlloc, p);
        return iterator(p);
    }();
};

// non-member functions
template < typename T, typename Alloc >
std::ostream& operator<<(std::ostream& os, const ForwardList<T, Alloc>& list)
{
    if (list.empty())
    {
        std::cout << "()";
        return os;
    }
    os << "(";
    for (auto i = list.cbegin(); i != list.cend(); i++)
    {
        auto nextIter = i;
        std::advance(nextIter, 1);
        if (nextIter == list.end())
            os << *i;    
        else
            os << *i << ", ";
    }
    os << ")";
    return os;
}

#endif // FORWARD_LIST_HPP_INCLUDED