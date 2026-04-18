#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

#ifndef SYS_PROG_B_PLUS_TREE_H
#define SYS_PROG_B_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BP_tree final : private compare //EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;
    inline bool keys_equal(const tkey& lhs, const tkey& rhs) const;

    // endregion comparators declaration

    struct bptree_node_base
    {
        bool _is_terminate;

        bptree_node_base() noexcept;
        virtual ~bptree_node_base() =default;
    };

    struct bptree_node_term : public bptree_node_base
    {
        bptree_node_term* _next = nullptr;

        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bptree_node_term() noexcept { _is_terminate = true; }
    };

    struct bptree_node_middle : public bptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bptree_node_middle() noexcept { _is_terminate = false; }
    };

    pp_allocator<value_type> _allocator;
    bptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;
    pp_allocator<bptree_node_middle> get_node_allocator_middle() const noexcept;
    pp_allocator<bptree_node_term> get_node_allocator_term() const noexcept;

    static value_type& as_value(tree_data_type& x) noexcept;
    static const value_type& as_value(const tree_data_type& x) noexcept;

    static bool iterators_are_equal(const bptree_node_term* _node, const size_t _index,
                                    const bptree_node_term* other_node, const size_t other_index);

    template<typename node_ptr>
    static void increment_iterator(node_ptr& node,
        size_t& index);

    bptree_node_term* get_leftmost_leaf();
    size_t find_key_index(bptree_node_middle* node, const tkey& key);
    size_t find_key_index(bptree_node_term* node, const tkey& key);
    size_t upper_bound_key_index(bptree_node_middle* node, const tkey& key);
    size_t upper_bound_key_index(bptree_node_term* node, const tkey& key);

    bptree_node_middle* make_node_middle();
    bptree_node_term* make_node_term();
    void delete_node_middle(bptree_node_middle* node);
    void delete_node_term(bptree_node_term* node);
    void destroy_tree(bptree_node_base* node);

    size_t get_keys_size(bptree_node_base* node);
    void split_child(bptree_node_middle* parent, size_t child_index);
    void insert_bottom_up(bptree_node_base* node, tree_data_type data);

    void borrow_from_left(bptree_node_middle* parent, size_t child_index);
    void borrow_from_right(bptree_node_middle* parent, size_t child_index);
    void merge_children(bptree_node_middle* parent, size_t left_child_index);
    void ensure_child_has_enough_keys(bptree_node_middle* parent, size_t& child_index);

    bool try_erase_from_node(bptree_node_base* node, const tkey& key);

public:

    // region constructors declaration

    explicit BP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BP_tree(const BP_tree& other);

    BP_tree(BP_tree&& other) noexcept;

    BP_tree& operator=(const BP_tree& other);

    BP_tree& operator=(BP_tree&& other) noexcept;

    ~BP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bptree_iterator;
    class bptree_const_iterator;

    class bptree_iterator final
    {
        bptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_iterator;

        friend class BP_tree;
        friend class bptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_iterator(bptree_node_term* node = nullptr, size_t index = 0);

    };

    class bptree_const_iterator final
    {
        const bptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_const_iterator;

        friend class BP_tree;
        friend class bptree_iterator;

        bptree_const_iterator(const bptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_const_iterator(const bptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class bptree_iterator;
    friend class bptree_const_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue& at(const tkey&);
    const tvalue& at(const tkey&) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    // endregion element access declaration
    // region iterator begins declaration

    bptree_iterator begin();
    bptree_iterator end();

    bptree_const_iterator begin() const;
    bptree_const_iterator end() const;

    bptree_const_iterator cbegin() const;
    bptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bptree_iterator find(const tkey& key);
    bptree_const_iterator find(const tkey& key) const;

    bptree_iterator lower_bound(const tkey& key);
    bptree_const_iterator lower_bound(const tkey& key) const;

    bptree_iterator upper_bound(const tkey& key);
    bptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bptree_iterator insert_or_assign(const tree_data_type& data);
    bptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bptree_iterator erase(bptree_iterator pos);
    bptree_iterator erase(bptree_const_iterator pos);

    bptree_iterator erase(bptree_iterator beg, bptree_iterator en);
    bptree_iterator erase(bptree_const_iterator beg, bptree_const_iterator en);


    bptree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_pairs(const BP_tree::tree_data_type &lhs,
                                                     const BP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::keys_equal(const tkey& lhs, const tkey& rhs) const
{
    return !compare_keys(lhs, rhs) && !compare_keys(rhs, lhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_base::bptree_node_base() noexcept = default;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::value_type> BP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::bptree_node_middle> BP_tree<tkey, tvalue, compare, t>::get_node_allocator_middle() const noexcept
{
    return pp_allocator<bptree_node_middle>(_allocator);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term> BP_tree<tkey, tvalue, compare, t>::get_node_allocator_term() const noexcept
{
    return pp_allocator<bptree_node_term>(_allocator);
}



template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::value_type&
BP_tree<tkey, tvalue, compare, t>::as_value(tree_data_type& x) noexcept
{
    return reinterpret_cast<value_type&>(x);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BP_tree<tkey, tvalue, compare, t>::value_type&
BP_tree<tkey, tvalue, compare, t>::as_value(const tree_data_type& x) noexcept
{
    return reinterpret_cast<const value_type&>(x);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_iterator::operator*() const noexcept
{
    return BP_tree::as_value(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::pointer BP_tree<tkey, tvalue, compare, t>::bptree_iterator
::operator->() const noexcept
{
    return &(BP_tree::as_value(_node->_data[_index]));
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::iterators_are_equal(const bptree_node_term* _node, const size_t _index,
    const bptree_node_term* other_node, const size_t other_index)
{
    return _index == other_index && _node == other_node;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename node_ptr>
void BP_tree<tkey, tvalue, compare, t>::increment_iterator(node_ptr& _node,
    size_t& _index)
{
    if (_node == nullptr)
        return;

    if (_index + 1 < _node->_data.size())
    {
        _index++;
    }
    else
    {
        _node = _node->_next;
        _index = 0;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self & BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++()
{
    BP_tree::increment_iterator(_node, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator==(const self &other) const noexcept
{
    return BP_tree::iterators_are_equal(_node, _index, other._node, other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::current_node_keys_count() const noexcept
{
    return _node == nullptr ? 0 : _node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::bptree_iterator(bptree_node_term *node, size_t index)
    : _node(node), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_iterator &it) noexcept
    : _node(it._node), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator*() const noexcept
{
    return BP_tree::as_value(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::pointer BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator->() const noexcept
{
    return &(BP_tree::as_value(_node->_data[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self & BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++()
{
    BP_tree::increment_iterator(_node, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator==(const self &other) const noexcept
{
    return BP_tree::iterators_are_equal(_node, _index, other._node, other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node == nullptr ? 0 : _node->_data.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_node_term *node, size_t index)
    : _node(node), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey &key)
{
    auto it = find(key);
    if (it == end())
        throw std::out_of_range("BP_tree::at");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey &key) const
{
    auto it = find(key);
    if (it == end())
        throw std::out_of_range("BP_tree::at");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](const tkey &key)
{
    return emplace(key, tvalue{}).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](tkey &&key)
{
    return emplace(std::move(key), tvalue{}).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(
    const tree_data_type &data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(pp_allocator<value_type> alloc, const compare& cmp)
    : BP_tree(cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BP_tree<tkey, tvalue, compare, t>::BP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BP_tree(cmp, alloc)
{
    for (; begin != end; begin++)
    {
        insert(tree_data_type(begin->first, begin->second));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BP_tree(data.begin(), data.end(), cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const BP_tree& other)
    : compare(other),
    _allocator(other._allocator.select_on_container_copy_construction()),
    _root(nullptr),
    _size(0)
{
    for (auto it = other.begin(); it != other.end(); it++)
    {
        insert(tree_data_type(it->first, it->second));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(BP_tree&& other) noexcept
    : compare(std::move(other)),
    _allocator(std::move(other._allocator)),
    _root(other._root),
    _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(const BP_tree& other)
{
    if (this != &other)
    {
        *this = BP_tree(other);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(BP_tree&& other) noexcept
{
    if (this != &other)
    {
        clear();
        static_cast<compare&>(*this) = std::move(static_cast<compare&>(other));
        _allocator = std::move(other._allocator);
        _root = other._root;
        _size = other._size;
        other._root = nullptr;
        other._size = 0;
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::~BP_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* BP_tree<tkey, tvalue, compare, t>::get_leftmost_leaf()
{
    if (_root == nullptr)
        return nullptr;

    bptree_node_base* child = _root;

    while (true)
    {
        if (child->_is_terminate)
        {
            return static_cast<bptree_node_term*>(child);
        }

        bptree_node_middle* mid = static_cast<bptree_node_middle*>(child);
        child = mid->_pointers[0];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr)
        return bptree_iterator();
    return bptree_iterator(get_leftmost_leaf(), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::end()
{
    return bptree_iterator(nullptr, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return bptree_const_iterator(const_cast<BP_tree*>(this)->begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cend() const
{
    return bptree_const_iterator(const_cast<BP_tree*>(this)->end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::find_key_index(bptree_node_middle* node, const tkey& key)
{
    size_t left = 0;
    size_t right = node->_keys.size();
    while (left < right)
    {
        size_t mid = (left + right) / 2;
        if (keys_equal(node->_keys[mid], key))
            return mid;
        else if (compare_keys(node->_keys[mid], key))
            left = mid + 1;
        else
            right = mid;
    }
    return left;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::find_key_index(bptree_node_term* node, const tkey& key)
{
    size_t left = 0;
    size_t right = node->_data.size();
    while (left < right)
    {
        size_t mid = (left + right) / 2;
        if (keys_equal(node->_data[mid].first, key))
            return mid;
        else if (compare_keys(node->_data[mid].first, key))
            left = mid + 1;
        else
            right = mid;
    }
    return left;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::upper_bound_key_index(bptree_node_middle* node, const tkey& key)
{
    size_t idx = find_key_index(node, key);
    if (idx < node->_keys.size() && keys_equal(node->_keys[idx], key)) return idx + 1;
    return idx;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::upper_bound_key_index(bptree_node_term* node, const tkey& key)
{
    size_t idx = find_key_index(node, key);
    if (idx < node->_data.size() && keys_equal(node->_data[idx].first, key)) return idx + 1;
    return idx;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (_root == nullptr)
        return end();

    bptree_node_base* cur = _root;

    while (true)
    {
        if (cur->_is_terminate)
        {
            bptree_node_term* term = static_cast<bptree_node_term*>(cur);
            size_t i = find_key_index(term, key);
            if (i < term->_data.size() && keys_equal(term->_data[i].first, key))
            {
                return bptree_iterator(term, i);
            }
            return end();
        }

        bptree_node_middle* mid = static_cast<bptree_node_middle*>(cur);
        cur = mid->_pointers[upper_bound_key_index(mid, key)];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return bptree_const_iterator(const_cast<BP_tree*>(this)->find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr)
        return end();

    bptree_node_base* cur = _root;

    while (true)
    {
        if (cur->_is_terminate)
        {
            bptree_node_term* term = static_cast<bptree_node_term*>(cur);
            size_t idx = find_key_index(term, key);
            if (idx == term->_data.size())
            {
                if (term->_next == nullptr) return end();
                return bptree_iterator(term->_next, 0);
            }

            return bptree_iterator(term, idx);
        }

        bptree_node_middle* mid = static_cast<bptree_node_middle*>(cur);
        cur = mid->_pointers[upper_bound_key_index(mid, key)];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    return bptree_const_iterator(const_cast<BP_tree*>(this)->lower_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr)
        return end();

    bptree_node_base* cur = _root;

    while (true)
    {
        if (cur->_is_terminate)
        {
            bptree_node_term* term = static_cast<bptree_node_term*>(cur);
            size_t idx = upper_bound_key_index(term, key);
            if (idx == term->_data.size())
            {
                if (term->_next == nullptr) return end();
                return bptree_iterator(term->_next, 0);
            }

            return bptree_iterator(term, idx);
        }

        bptree_node_middle* mid = static_cast<bptree_node_middle*>(cur);
        cur = mid->_pointers[upper_bound_key_index(mid, key)];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    return bptree_const_iterator(const_cast<BP_tree*>(this)->upper_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_middle* 
BP_tree<tkey, tvalue, compare, t>::make_node_middle()
{
    return get_node_allocator_middle().template new_object<bptree_node_middle>();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_node_term* 
BP_tree<tkey, tvalue, compare, t>::make_node_term()
{
    return get_node_allocator_term().template new_object<bptree_node_term>();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::delete_node_middle(bptree_node_middle* node)
{
    if (node != nullptr)
        return get_node_allocator_middle().template delete_object<bptree_node_middle>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::delete_node_term(bptree_node_term* node)
{
    if (node != nullptr)
        return get_node_allocator_term().template delete_object<bptree_node_term>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::destroy_tree(bptree_node_base* node)
{
    if (node == nullptr)
        return;

    if (node->_is_terminate)
    {
        delete_node_term(static_cast<bptree_node_term*>(node));
    }
    else
    {
        bptree_node_middle* mid = static_cast<bptree_node_middle*>(node);
        for (auto* child : mid->_pointers)
        {
            destroy_tree(child);
        }
        delete_node_middle(mid);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    destroy_tree(_root);
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::get_keys_size(bptree_node_base* node)
{
    if (node == nullptr) return 0;

    if (node->_is_terminate)
        return static_cast<bptree_node_term*>(node)->_data.size();
    
    return static_cast<bptree_node_middle*>(node)->_keys.size();
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_child(bptree_node_middle* parent, size_t child_index)
{
    bptree_node_base* left = parent->_pointers[child_index];
    size_t med_idx = minimum_keys_in_node + 1;

    if (left->_is_terminate)
    {
        bptree_node_term* left_term = static_cast<bptree_node_term*>(left);
        bptree_node_term* right_term = make_node_term();

        right_term->_next = left_term->_next;
        left_term->_next = right_term;

        for (size_t i = med_idx; i < left_term->_data.size(); i++)
        {
            right_term->_data.push_back(std::move(left_term->_data[i]));
        }
        left_term->_data.erase(left_term->_data.begin() + med_idx, left_term->_data.end());

        parent->_pointers.insert(parent->_pointers.begin() + (child_index + 1), right_term);
        parent->_keys.insert(parent->_keys.begin() + child_index, right_term->_data[0].first);
    }
    else
    {
        bptree_node_middle* left_mid = static_cast<bptree_node_middle*>(left);
        bptree_node_middle* right_mid = make_node_middle();
        tkey median = std::move(left_mid->_keys[med_idx]);

        for (size_t i = med_idx + 1; i < left_mid->_keys.size(); i++)
        {
            right_mid->_keys.push_back(std::move(left_mid->_keys[i]));
        }
        left_mid->_keys.erase(left_mid->_keys.begin() + med_idx, left_mid->_keys.end());

        for (size_t i = med_idx + 1; i < left_mid->_pointers.size(); i++)
        {
            right_mid->_pointers.push_back(left_mid->_pointers[i]);
        }

        left_mid->_pointers.erase(left_mid->_pointers.begin() + (med_idx + 1), left_mid->_pointers.end());

        parent->_pointers.insert(parent->_pointers.begin() + (child_index + 1), right_mid);
        parent->_keys.insert(parent->_keys.begin() + child_index, std::move(median));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::insert_bottom_up(bptree_node_base* node, tree_data_type data)
{

    if (node->_is_terminate)
    {
        bptree_node_term* term = static_cast<bptree_node_term*>(node);
        size_t idx = find_key_index(term, data.first);
        term->_data.insert(term->_data.begin() + idx, std::move(data));
        return;
    }

    bptree_node_middle* mid = static_cast<bptree_node_middle*>(node);
    size_t i = upper_bound_key_index(mid, data.first);

    insert_bottom_up(mid->_pointers[i], std::move(data));

    if (get_keys_size(mid->_pointers[i]) > maximum_keys_in_node)
    {
        split_child(mid, i);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);

    auto existing = find(data.first);
    if (existing != end())
    {
        return { existing, false };
    }

    tkey key = data.first;

    if (_root == nullptr)
    {
        bptree_node_term* term = make_node_term();
        term->_data.push_back(std::move(data));
        term->_next = nullptr;
        _size++;
        _root = term;
        return { begin(), true };
    }

    insert_bottom_up(_root, std::move(data));

    if (get_keys_size(_root) > maximum_keys_in_node)
    {
        bptree_node_middle* new_root = make_node_middle();
        new_root->_pointers.push_back(_root);
        _root = new_root;
        split_child(new_root, 0);
    }

    _size++;
    return { find(key), true };
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto it = find(data.first);
    if (it != end())
    {
        it->second = data.second;
        return it;
    }
    return insert(data).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto it = find(data.first);
    if (it != end())
    {
        it->second = std::move(data.second);
        return it;
    }
    return insert(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator pos)
{
    if (pos == end())
        return end();
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator pos)
{
    if (pos == cend())
        return end();
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator beg, bptree_iterator en)
{
    while (beg != en)
    {
        beg = erase(beg);
    }
    return beg;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator beg, bptree_const_iterator en)
{
    auto first = (beg == cend()) ? end() : find(beg->first);
    auto last = (en == cend()) ? end() : find(en->first);
    return erase(first, last);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::borrow_from_left(bptree_node_middle* parent, size_t child_index)
{
    bptree_node_middle* child_mid = dynamic_cast<bptree_node_middle*>(parent->_pointers[child_index]);

    if (child_mid != nullptr)
    {
        bptree_node_middle* left_mid = dynamic_cast<bptree_node_middle*>(parent->_pointers[child_index - 1]);
        child_mid->_keys.insert(child_mid->_keys.begin(), std::move(parent->_keys[child_index - 1]));

        child_mid->_pointers.insert(child_mid->_pointers.begin(), left_mid->_pointers.back());
        left_mid->_pointers.pop_back();

        parent->_keys[child_index - 1] = std::move(left_mid->_keys.back());
        left_mid->_keys.pop_back();
    }
    else
    {
        bptree_node_term* child_term = dynamic_cast<bptree_node_term*>(parent->_pointers[child_index]);
        bptree_node_term* left_term = dynamic_cast<bptree_node_term*>(parent->_pointers[child_index - 1]);

        child_term->_data.insert(child_term->_data.begin(), std::move(left_term->_data.back()));
        parent->_keys[child_index - 1] = child_term->_data.front().first;
        left_term->_data.pop_back();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::borrow_from_right(bptree_node_middle* parent, size_t child_index)
{

    if (parent->_pointers[child_index]->_is_terminate)
    {
        bptree_node_term* child_term = static_cast<bptree_node_term*>(parent->_pointers[child_index]);
        bptree_node_term* right_term = static_cast<bptree_node_term*>(parent->_pointers[child_index + 1]);

        child_term->_data.push_back(std::move(right_term->_data.front()));
        right_term->_data.erase(right_term->_data.begin());
        parent->_keys[child_index] = right_term->_data.front().first;
    }
    else
    {
        bptree_node_middle* child_mid = static_cast<bptree_node_middle*>(parent->_pointers[child_index]);
        bptree_node_middle* right_mid = static_cast<bptree_node_middle*>(parent->_pointers[child_index + 1]);
        child_mid->_keys.push_back(std::move(parent->_keys[child_index]));

        child_mid->_pointers.push_back(right_mid->_pointers.front());
        right_mid->_pointers.erase(right_mid->_pointers.begin());

        parent->_keys[child_index] = std::move(right_mid->_keys.front());
        right_mid->_keys.erase(right_mid->_keys.begin());
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::merge_children(bptree_node_middle* parent, size_t left_child_index)
{
    if (parent->_pointers[left_child_index]->_is_terminate)
    {
        bptree_node_term* left_term = static_cast<bptree_node_term*>(parent->_pointers[left_child_index]);
        bptree_node_term* right = static_cast<bptree_node_term*>(parent->_pointers[left_child_index + 1]);

        for (auto& key : right->_data)
        {
            left_term->_data.push_back(std::move(key));
        }

        parent->_keys.erase(parent->_keys.begin() + left_child_index);
        parent->_pointers.erase(parent->_pointers.begin() + (left_child_index + 1));
        left_term->_next = right->_next;
        delete_node_term(right);
    }
    else
    {
        bptree_node_middle* left = static_cast<bptree_node_middle*>(parent->_pointers[left_child_index]);
        bptree_node_middle* right = static_cast<bptree_node_middle*>(parent->_pointers[left_child_index + 1]);

        left->_keys.push_back(parent->_keys[left_child_index]);

        for (auto& key : right->_keys)
        {
            left->_keys.push_back(std::move(key));
        }

        for (auto ptr : right->_pointers)
        {
            left->_pointers.push_back(ptr);
        }

        parent->_keys.erase(parent->_keys.begin() + left_child_index);
        parent->_pointers.erase(parent->_pointers.begin() + (left_child_index + 1));

        delete_node_middle(right);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::ensure_child_has_enough_keys(bptree_node_middle* parent, size_t& child_index)
{
    if (get_keys_size(parent->_pointers[child_index]) >= minimum_keys_in_node)
        return;

    if (child_index > 0 && get_keys_size(parent->_pointers[child_index - 1]) > minimum_keys_in_node)
    {
        borrow_from_left(parent, child_index);
        return;
    }

    if (child_index + 1 < parent->_pointers.size()
        && get_keys_size(parent->_pointers[child_index + 1]) > minimum_keys_in_node)
    {
        borrow_from_right(parent, child_index);
        return;
    }

    if (child_index + 1 < parent->_pointers.size())
    {
        merge_children(parent, child_index);
    }
    else
    {
        merge_children(parent, child_index - 1);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::try_erase_from_node(bptree_node_base* node, const tkey& key)
{
    if (node->_is_terminate)
    {
        bptree_node_term* node_term = static_cast<bptree_node_term*>(node);

        size_t idx = find_key_index(node_term, key);

        if (idx < node_term->_data.size() && keys_equal(node_term->_data[idx].first, key))
        {
            node_term->_data.erase(node_term->_data.begin() + idx);
            return true;
        }

        return false;
    }
    else
    {
        bptree_node_middle* node_mid = static_cast<bptree_node_middle*>(node);
        size_t idx = upper_bound_key_index(node_mid, key);
        bool res = try_erase_from_node(node_mid->_pointers[idx], key);
        if (res)
            ensure_child_has_enough_keys(node_mid, idx);
        return res;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    if (_root == nullptr)
        return end();

    if (!try_erase_from_node(_root, key))
        return end();

    --_size;


    if (_root->_is_terminate)
    {
        bptree_node_term* root_term = static_cast<bptree_node_term*>(_root);

        if (root_term->_data.empty())
        {
            _root = nullptr;
            delete_node_term(root_term);
        }
    }
    else
    {
        bptree_node_middle* root_mid = static_cast<bptree_node_middle*>(_root);
        if (root_mid->_keys.empty())
        {
            if (root_mid->_pointers.empty())
            {
                _root = nullptr;
            }
            else
            {
                _root = root_mid->_pointers[0];
            }

            delete_node_middle(root_mid);
        }
    }

    return lower_bound(key);
}

#endif