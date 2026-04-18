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

#ifndef SYS_PROG_BS_TREE_H
#define SYS_PROG_BS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BS_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    // TODO: Another restrictions
    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 3 * t - 1;
    static constexpr const size_t maximum_keys_in_root = 4 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;
    inline bool keys_equal(const tkey& lhs, const tkey& rhs) const;

    // endregion comparators declaration

    struct bstree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_root + 1> _keys;
        boost::container::static_vector<bstree_node*, maximum_keys_in_root + 2> _pointers;
        bstree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bstree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;
    pp_allocator<bstree_node> get_node_allocator() const noexcept;

    static value_type& as_value(tree_data_type& x) noexcept;
    static const value_type& as_value(const tree_data_type& x) noexcept;

    template<typename node_ptr_ptr>
    static void increment_iterator(std::stack<std::pair<node_ptr_ptr, size_t>>& path,
        size_t& index);

    template<typename node_ptr_ptr>
    static void decrement_iterator(std::stack<std::pair<node_ptr_ptr, size_t>>& path,
        size_t& index);

    template<typename node_ptr_ptr>
    static bool iterators_are_equal(const std::stack<std::pair<node_ptr_ptr, size_t>>& path,
        const size_t& index, const std::stack<std::pair<node_ptr_ptr, size_t>>& other_path,
        const size_t& other_index);

    std::stack<std::pair<bstree_node**, size_t>> get_leftmost_path();
    std::stack<std::pair<bstree_node**, size_t>> get_rightmost_path();
    bstree_node* make_node();
    void delete_node(bstree_node* node);
    void destroy_tree(bstree_node* node);
    bool try_lend_key_to_neighbour(bstree_node* parent, size_t child_index);
    void split_child(bstree_node* parent, size_t child_index);
    void split_root();
    void insert_bottom_up(bstree_node* node, tree_data_type data);

    size_t find_key_index(bstree_node* node, const tkey& key);
    size_t upper_bound_key_index(bstree_node* node, const tkey& key);
    
    tree_data_type& get_min_key(bstree_node* node);
    tree_data_type& get_max_key(bstree_node* node);

    void borrow_from_left(bstree_node* parent, size_t child_index);
    void borrow_from_right(bstree_node* parent, size_t child_index);
    void merge_3_children(bstree_node* parent, size_t child_index);
    void merge_2_children(bstree_node* parent, size_t left_child_index);
    void ensure_child_has_enough_keys(bstree_node* parent, size_t& child_index);

    bool try_erase_from_node(bstree_node* node, const tkey& key);
public:

    // region constructors declaration

    explicit BS_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BS_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BS_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BS_tree(const BS_tree& other);

    BS_tree(BS_tree&& other) noexcept;

    BS_tree& operator=(const BS_tree& other);

    BS_tree& operator=(BS_tree&& other) noexcept;

    ~BS_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bstree_iterator;
    class bstree_reverse_iterator;
    class bstree_const_iterator;
    class bstree_const_reverse_iterator;

    class bstree_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);

    };

    class bstree_const_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_const_iterator(const bstree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_const_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    class bstree_reverse_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_reverse_iterator;

        friend class BS_tree;
        friend class bstree_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_reverse_iterator(const bstree_iterator& it) noexcept;
        operator bstree_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_reverse_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);
    };

    class bstree_const_reverse_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_reverse_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_iterator;

        bstree_const_reverse_iterator(const bstree_reverse_iterator& it) noexcept;
        operator bstree_const_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_const_reverse_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class bstree_iterator;
    friend class bstree_const_iterator;
    friend class bstree_reverse_iterator;
    friend class bstree_const_reverse_iterator;

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

    bstree_iterator begin();
    bstree_iterator end();

    bstree_const_iterator begin() const;
    bstree_const_iterator end() const;

    bstree_const_iterator cbegin() const;
    bstree_const_iterator cend() const;

    bstree_reverse_iterator rbegin();
    bstree_reverse_iterator rend();

    bstree_const_reverse_iterator rbegin() const;
    bstree_const_reverse_iterator rend() const;

    bstree_const_reverse_iterator crbegin() const;
    bstree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bstree_iterator find(const tkey& key);
    bstree_const_iterator find(const tkey& key) const;

    bstree_iterator lower_bound(const tkey& key);
    bstree_const_iterator lower_bound(const tkey& key) const;

    bstree_iterator upper_bound(const tkey& key);
    bstree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bstree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bstree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bstree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bstree_iterator insert_or_assign(const tree_data_type& data);
    bstree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bstree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bstree_iterator erase(bstree_iterator pos);
    bstree_iterator erase(bstree_const_iterator pos);

    bstree_iterator erase(bstree_iterator beg, bstree_iterator en);
    bstree_iterator erase(bstree_const_iterator beg, bstree_const_iterator en);


    bstree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BS_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BS_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BS_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_pairs(const BS_tree::tree_data_type &lhs,
                                                     const BS_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::keys_equal(const tkey& lhs, const tkey& rhs) const
{
    return !compare_keys(lhs, rhs) && !compare_keys(rhs, lhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_node::bstree_node() noexcept = default;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BS_tree<tkey, tvalue, compare, t>::value_type> BS_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BS_tree<tkey, tvalue, compare, t>::bstree_node> BS_tree<tkey, tvalue, compare, t>::
get_node_allocator() const noexcept
{
    return pp_allocator<bstree_node>(_allocator);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::value_type& BS_tree<tkey, tvalue, compare, t>::
as_value(tree_data_type& x) noexcept
{
    return reinterpret_cast<value_type&>(x);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const typename BS_tree<tkey, tvalue, compare, t>::value_type& BS_tree<tkey, tvalue, compare, t>::
as_value(const tree_data_type& x) noexcept
{
    return reinterpret_cast<const value_type&>(x);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_iterator::operator*() const noexcept
{
    return BS_tree::as_value((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::pointer BS_tree<tkey, tvalue, compare, t>::bstree_iterator
::operator->() const noexcept
{
    return &(BS_tree::as_value((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename node_ptr_ptr>
void BS_tree<tkey, tvalue, compare, t>::increment_iterator(std::stack<std::pair<node_ptr_ptr, size_t>>& path,
    size_t& index)
{
    if (path.empty())
        return;

    if ((*path.top().first)->_pointers.empty())
    {
        if (index + 1 < (*path.top().first)->_keys.size())
        {
            index++;
            return;
        }

        auto path_tmp = path;
        auto child = path_tmp.top();
        path_tmp.pop();

        while (!path_tmp.empty())
        {
            size_t next_in_parent = child.second;
            if (next_in_parent < (*path_tmp.top().first)->_keys.size())
            {
                path = path_tmp;
                index = next_in_parent;
                return;
            }

            child = path_tmp.top();
            path_tmp.pop();
        }

        index = (*path.top().first)->_keys.size();
        return;
    }

    auto kid = &((*path.top().first)->_pointers[index + 1]);
    path.push({ kid, index + 1 });
    while (!((*kid)->_pointers.empty()))
    {
        kid = &((*kid)->_pointers[0]);
        path.push({ kid, 0 });
    }

    index = 0;
    return;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename node_ptr_ptr>
void BS_tree<tkey, tvalue, compare, t>::decrement_iterator(std::stack<std::pair<node_ptr_ptr, size_t>>& path,
    size_t& index)
{
    if (path.empty())
        return;

    if (index == (*path.top().first)->_keys.size())
    {
        if (!(*path.top().first)->_keys.empty())
            index--;
        return;
    }

    if ((*path.top().first)->_pointers.empty())
    {
        if (index > 0)
        {
            index--;
            return;
        }

        auto path_tmp = path;
        auto child = path_tmp.top();
        path_tmp.pop();

        while (!path_tmp.empty())
        {
            if (child.second > 0)
            {
                path = path_tmp;
                index = child.second - 1;
                return;
            }

            child = path_tmp.top();
            path_tmp.pop();
        }

        index = 0;
        return;
    }

    auto kid = &((*path.top().first)->_pointers[index]);
    path.push({ kid, index });
    while (!((*kid)->_pointers.empty()))
    {
        size_t child_index = (*kid)->_pointers.size() - 1;
        kid = &((*kid)->_pointers[child_index]);
        path.push({ kid, child_index });
    }

    index = (*kid)->_keys.size() - 1;
    return;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename node_ptr_ptr>
bool BS_tree<tkey, tvalue, compare, t>::iterators_are_equal(const std::stack<std::pair<node_ptr_ptr, size_t>>& path,
    const size_t& index, const std::stack<std::pair<node_ptr_ptr, size_t>>& other_path,
    const size_t& other_index)
{
    if (index != other_index)
        return false;

    if (path.size() != other_path.size())
        return false;

    if (path.empty())
        return true;

    return (path.top().first == other_path.top().first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self & BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator++()
{
    BS_tree::increment_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self & BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator--()
{
    BS_tree::decrement_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator==(const self &other) const noexcept
{
    return BS_tree::iterators_are_equal(_path, _index, other._path, other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : (_path.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || _index >= (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::bstree_iterator(
    const std::stack<std::pair<bstree_node **, size_t>> &path, size_t index)
    : _path(path), _index(index)
{ }

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(const bstree_iterator &it) noexcept
    : _index(it._index)
{
    std::stack<std::pair<bstree_node* const*, size_t>> tmp;
    auto st = it._path;

    while (!st.empty())
    {
        tmp.push({ st.top().first, st.top().second });
        st.pop();
    }

    while (!tmp.empty())
    {
        _path.push({ tmp.top().first, tmp.top().second });
        tmp.pop();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator*() const noexcept
{
    return BS_tree::as_value((*_path.top().first)->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator->() const noexcept
{
    return &(BS_tree::as_value((*_path.top().first)->_keys[_index]));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator++()
{
    BS_tree::increment_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator--()
{
    BS_tree::decrement_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator==(const self &other) const noexcept
{
    return BS_tree::iterators_are_equal(_path, _index, other._path, other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : (_path.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::current_node_keys_count() const noexcept
{
    return _path.empty() ? 0 : (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::is_terminate_node() const noexcept
{
    return _path.empty() || _index >= (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(
    const std::stack<std::pair<bstree_node* const*, size_t>>& path, size_t index)
    : _path(path), _index(index)
{}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(const bstree_iterator& it) noexcept
    : _path(it._path), _index(it._index)
{}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_iterator() const noexcept
{
    return bstree_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator*() const noexcept
{
    bstree_iterator it = *this;
    --it;
    return *it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator->() const noexcept
{
    return &(operator*());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator++()
{
    BS_tree::decrement_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator--()
{
    BS_tree::increment_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator==(const self &other) const noexcept
{
    return BS_tree::iterators_are_equal(_path, _index, other._path, other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::depth() const noexcept
{
    bstree_iterator it = *this;
    --it;
    return it.depth();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::current_node_keys_count() const noexcept
{
    bstree_iterator it = *this;
    --it;
    return it.current_node_keys_count();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::is_terminate_node() const noexcept
{
    bstree_iterator it = *this;
    --it;
    return it.is_terminate_node();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::index() const noexcept
{
    bstree_iterator it = *this;
    --it;
    return it.index();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(
    const std::stack<std::pair<bstree_node**, size_t>>& path, size_t index)
    : _path(path), _index(index)
{}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const bstree_reverse_iterator &it) noexcept
{
    std::stack<std::pair<bstree_node* const*, size_t>> tmp;
    auto st = it._path;

    while (!st.empty())
    {
        tmp.push({ st.top().first, st.top().second });
        st.pop();
    }

    while (!tmp.empty())
    {
        _path.push({ tmp.top().first, tmp.top().second });
        tmp.pop();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator() const noexcept
{
    return bstree_const_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator*() const noexcept
{
    bstree_const_iterator it = *this;
    --it;
    return *it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator->() const noexcept
{
    return &(operator*());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator++()
{
    BS_tree::decrement_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator--()
{
    BS_tree::increment_iterator(_path, _index);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator==(const self &other) const noexcept
{
    return BS_tree::iterators_are_equal(_path, _index, other._path, other._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::depth() const noexcept
{
    bstree_const_iterator it = *this;
    --it;
    return it.depth();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    bstree_const_iterator it = *this;
    --it;
    return it.current_node_keys_count();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::is_terminate_node() const noexcept
{
    bstree_const_iterator it = *this;
    --it;
    return it.is_terminate_node();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::index() const noexcept
{
    bstree_const_iterator it = *this;
    --it;
    return it.index();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const std::stack<std::pair<bstree_node* const*, size_t>>& path, size_t index)
    : _path(path), _index(index)
{}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(pp_allocator<value_type> alloc, const compare& comp)
    : BS_tree(comp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BS_tree<tkey, tvalue, compare, t>::BS_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BS_tree(cmp, alloc)
{
    for (; begin != end; begin++)
    {
        insert(tree_data_type(begin->first, begin->second));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BS_tree(data.begin(), data.end(), cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const BS_tree& other)
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
BS_tree<tkey, tvalue, compare, t>::BS_tree(BS_tree&& other) noexcept
    : compare(std::move(other)),
    _allocator(std::move(other._allocator)),
    _root(other._root),
    _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(const BS_tree& other)
{
    if (this != &other)
    {
        *this = BS_tree(other);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(BS_tree&& other) noexcept
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
BS_tree<tkey, tvalue, compare, t>::~BS_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end())
        throw std::out_of_range("BS_tree::at");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end())
        throw std::out_of_range("BS_tree::at");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(key, tvalue{}).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::move(key), tvalue{}).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::stack<std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_node**, size_t>>
BS_tree<tkey, tvalue, compare, t>::get_leftmost_path()
{
    std::stack<std::pair<bstree_node**, size_t>> path;

    if (_root == nullptr)
        return path;

    bstree_node** kid = &_root;
    path.push({ kid, 0 });
    while (!((*kid)->_pointers.empty()))
    {
        kid = &((*kid)->_pointers[0]);
        path.push({ kid, 0 });
    }
    return path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::stack<std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_node**, size_t>>
BS_tree<tkey, tvalue, compare, t>::get_rightmost_path()
{
    std::stack<std::pair<bstree_node**, size_t>> path;

    if (_root == nullptr)
        return path;

    bstree_node** kid = &_root;
    path.push({ kid, 0 });
    while (!((*kid)->_pointers.empty()))
    {
        size_t child_index = (*kid)->_pointers.size() - 1;
        kid = &((*kid)->_pointers[child_index]);
        path.push({ kid, child_index });
    }
    return path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr)
        return bstree_iterator();
    return bstree_iterator(get_leftmost_path(), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::end()
{
    if (_root == nullptr)
        return bstree_iterator();

    auto path = get_rightmost_path();
    return bstree_iterator(path, (*path.top().first)->_keys.size());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cend() const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin()
{
    return bstree_reverse_iterator(end());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend()
{
    return bstree_reverse_iterator(begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crbegin() const
{
    return bstree_const_reverse_iterator(const_cast<BS_tree*>(this)->rbegin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crend() const
{
    return bstree_const_reverse_iterator(const_cast<BS_tree*>(this)->rend());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    if (_root == nullptr)
        return end();

    std::stack<std::pair<bstree_node**, size_t>> path;
    bstree_node** cur = &_root;
    path.push({ cur, 0 });

    while (true)
    {
        bstree_node* node = *cur;
        size_t i = find_key_index(node, key);

        if (i < node->_keys.size() && keys_equal(node->_keys[i].first, key))
            return bstree_iterator(path, i);

        if (node->_pointers.empty())
            return end();

        cur = &(node->_pointers[i]);
        path.push({ cur, i });
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr)
        return end();

    std::stack<std::pair<bstree_node**, size_t>> path;
    std::stack<std::pair<bstree_node**, size_t>> best_path;
    size_t best_index = 0;
    bool found_best = false;
    bstree_node** cur = &_root;
    path.push({ cur, 0 });

    while (true)
    {
        bstree_node* node = *cur;
        size_t i = find_key_index(node, key);

        if (i < node->_keys.size())
        {
            if (keys_equal(node->_keys[i].first, key))
                return bstree_iterator(path, i);

            best_path = path;
            best_index = i;
            found_best = true;
        }

        if (node->_pointers.empty())
            return found_best ? bstree_iterator(best_path, best_index) : end();

        cur = &(node->_pointers[i]);
        path.push({ cur, i });
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->lower_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    if (_root == nullptr)
        return end();

    std::stack<std::pair<bstree_node**, size_t>> path;
    std::stack<std::pair<bstree_node**, size_t>> best_path;
    size_t best_index = 0;
    bool found_best = false;
    bstree_node** cur = &_root;
    path.push({ cur, 0 });

    while (true)
    {
        bstree_node* node = *cur;
        size_t i = upper_bound_key_index(node, key);

        if (i < node->_keys.size())
        {
            best_path = path;
            best_index = i;
            found_best = true;
        }

        if (node->_pointers.empty())
            return found_best ? bstree_iterator(best_path, best_index) : end();

        cur = &(node->_pointers[i]);
        path.push({ cur, i });
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    return bstree_const_iterator(const_cast<BS_tree*>(this)->upper_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_node* BS_tree<tkey, tvalue, compare, t>::make_node()
{
    return get_node_allocator().template new_object<bstree_node>();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::delete_node(bstree_node* node)
{
    if (node != nullptr)
        return get_node_allocator().template delete_object<bstree_node>(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::destroy_tree(bstree_node* node)
{
    if (node == nullptr)
        return;

    for (auto* child : node->_pointers)
    {
        destroy_tree(child);
    }

    delete_node(node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    destroy_tree(_root);
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> 
BS_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> 
BS_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::borrow_from_left(bstree_node* parent, size_t child_index)
{
    bstree_node* child = parent->_pointers[child_index];
    bstree_node* left = parent->_pointers[child_index - 1];

    child->_keys.insert(child->_keys.begin(), std::move(parent->_keys[child_index - 1]));

    if (!left->_pointers.empty())
    {
        child->_pointers.insert(child->_pointers.begin(), left->_pointers.back());
        left->_pointers.pop_back();
    }

    parent->_keys[child_index - 1] = std::move(left->_keys.back());
    left->_keys.pop_back();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::borrow_from_right(bstree_node* parent, size_t child_index)
{
    bstree_node* child = parent->_pointers[child_index];
    bstree_node* right = parent->_pointers[child_index + 1];

    child->_keys.push_back(std::move(parent->_keys[child_index]));

    if (!right->_pointers.empty())
    {
        child->_pointers.push_back(right->_pointers.front());
        right->_pointers.erase(right->_pointers.begin());
    }

    parent->_keys[child_index] = std::move(right->_keys.front());
    right->_keys.erase(right->_keys.begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::try_lend_key_to_neighbour(bstree_node* parent, size_t child_index)
{
    if (child_index > 0 && parent->_pointers[child_index - 1]->_keys.size() < maximum_keys_in_node)
    {
        borrow_from_right(parent, child_index - 1);
        return true;
    }

    if (child_index < parent->_pointers.size() - 1
        && parent->_pointers[child_index + 1]->_keys.size() < maximum_keys_in_node)
    {
        borrow_from_left(parent, child_index + 1);
        return true;
    }

    return false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::split_child(bstree_node* parent, size_t child_index)
{
    bstree_node* mid = parent->_pointers[child_index];
    bstree_node* left;
    if (child_index > 0)
    {
        left = parent->_pointers[child_index - 1];
    }
    else
    {
        left = mid;
        mid = parent->_pointers[child_index + 1];
        child_index++;
    }

    bstree_node* right = make_node();

    size_t med1_index = minimum_keys_in_node + 1;
    size_t med2_index = med1_index * 2 - 1 - left->_keys.size();
    tree_data_type median1 = med1_index < left->_keys.size()
                            ? std::move(left->_keys[med1_index]);
                            : std::move(mid->_keys[med1_index - left->_keys.size()]);
    tree_data_type median2 = std::move(mid->_keys[med2_index]);

    for (size_t i = med2_index + 1; i < mid->_keys.size(); i++)
    {
        right->_keys.push_back(std::move(mid->_keys[i]));
    }
    mid->_keys.erase(mid->_keys.begin() + med2_index, mid->_keys.end());
    mid->_keys.insert(mid->_keys.begin(), std::move(parent->_keys[child_index - 1]));

    for (size_t i = left->_keys.size() - 1; i > med1_index; i--)
    {
        mid->_keys.insert(mid->_keys.begin(), left->_keys[i]);
    }

    left->_keys.erase(left->_keys.begin() + med1_index, left->_keys.end());

    if (!left->_pointers.empty())
    {
        for (size_t i = med2_index + 1; i < mid->_pointers.size(); i++)
        {
            right->_pointers.push_back(mid->_pointers[i]);
        }
        mid->_pointers.erase(mid->_pointers.begin() + (med2_index + 1), mid->_pointers.end());

        for (size_t i = left->_pointers.size() - 1; i > med1_index; i--)
        {
            mid->_pointers.insert(mid->_pointers.begin(), left->_pointers[i]);
        }
        left->_pointers.erase(left->_pointers.begin() + (med1_index + 1), left->_pointers.end());
    }

    parent->_pointers.insert(parent->_pointers.begin() + (child_index + 1), right);
    parent->_keys.insert(parent->_keys.begin() + child_index, std::move(median2));
    parent->_keys[child_index - 1] = std::move(median1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::split_root()
{
    bstree_node* left = _root;
    bstree_node* right = make_node();

    size_t med_index = minimum_keys_in_node + 1;
    tree_data_type median = std::move(left->_keys[med_index]);

    for (size_t i = med_index + 1; i < left->_keys.size(); i++)
    {
        right->_keys.push_back(std::move(left->_keys[i]));
    }
    left->_keys.erase(left->_keys.begin() + med_index, left->_keys.end());

    if (!left->_pointers.empty())
    {
        for (size_t i = med_index + 1; i < left->_pointers.size(); i++)
        {
            right->_pointers.push_back(left->_pointers[i]);
        }
        left->_pointers.erase(left->_pointers.begin() + (med_index + 1), left->_pointers.end());
    }

    bstree_node* new_root = make_node();
    _root = new_root;
    _root->_pointers.push_back(left);
    _root->_pointers.push_back(right);
    _root->_keys.push_back(std::move(median));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::insert_bottom_up(bstree_node* node, tree_data_type data)
{
    size_t i = find_key_index(node, data.first);

    if (node->_pointers.empty())
    {
        node->_keys.insert(node->_keys.begin() + i, std::move(data));
        return;
    }

    insert_bottom_up(node->_pointers[i], std::move(data));

    if (node->_pointers[i]->_keys.size() > maximum_keys_in_node)
    {
        if (!try_lend_key_to_neighbour(node, i))
            split_child(node, i);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> 
BS_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
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
        _root = make_node();
        _root->_keys.push_back(std::move(data));
        _size++;
        return { begin(), true };
    }

    insert_bottom_up(_root, std::move(data));

    if (_root->_keys.size() > maximum_keys_in_root)
    {
        split_root();
    }

    _size++;
    return { find(key), true };
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator 
BS_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator 
BS_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
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
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator pos)
{
    if (pos == end())
        return end();
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator pos)
{
    if (pos == cend())
        return end();
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator beg, bstree_iterator en)
{
    while (beg != en)
    {
        beg = erase(beg);
    }
    return beg;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator beg, bstree_const_iterator en)
{
    auto first = (beg == cend()) ? end() : find(beg->first);
    auto last = (en == cend()) ? end() : find(en->first);
    return erase(first, last);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::find_key_index(bstree_node* node, const tkey& key)
{
    size_t left = 0;
    size_t right = node->_keys.size();
    while (left < right)
    {
        size_t mid = (left + right) / 2;
        if (keys_equal(node->_keys[mid].first, key))
        {
            return mid;
        }
        else if (compare_keys(node->_keys[mid].first, key))
        {
            left = mid + 1;
        }
        else
        {
            right = mid;
        }
    }
    return left;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::upper_bound_key_index(bstree_node* node, const tkey& key)
{
    size_t idx = find_key_index(node, key);
    if (idx < node->_keys.size() && keys_equal(node->_keys[idx].first, key))
        return idx + 1;
    return idx;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::tree_data_type& BS_tree<tkey, tvalue, compare, t>::get_min_key(bstree_node* node)
{
    while (!node->_pointers.empty())
        node = node->_pointers.front();

    return node->_keys.front();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::tree_data_type& BS_tree<tkey, tvalue, compare, t>::get_max_key(bstree_node* node)
{
    while (!node->_pointers.empty())
        node = node->_pointers.back();

    return node->_keys.back();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::merge_2_children(bstree_node* parent, size_t left_child_index)
{
    bstree_node* left = parent->_pointers[left_child_index];
    bstree_node* right = parent->_pointers[left_child_index + 1];

    left->_keys.push_back(parent->_keys[left_child_index]);

    for (auto& key : right->_keys)
    {
        left->_keys.push_back(std::move(key));
    }

    for (auto& ptr : right->_pointers)
    {
        left->_pointers.push_back(ptr);
    }

    parent->_keys.erase(parent->_keys.begin() + left_child_index);
    parent->_pointers.erase(parent->_pointers.begin() + (left_child_index + 1));

    delete_node(right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::merge_3_children(bstree_node* parent, size_t mid_child_index)
{
    bstree_node* left = parent->_pointers[mid_child_index - 1];
    bstree_node* mid = parent->_pointers[mid_child_index];
    bstree_node* right = parent->_pointers[mid_child_index + 1];

    bool are_leaves = left->_pointers.size() == 0;

    size_t med_idx = (left->_keys.size() + mid->_keys.size() + right->_keys.size() + 2) / 2
                    - left->_keys.size() - 1;

    left->_keys.push_back(std::move(parent->_keys[mid_child_index - 1]));

    for (size_t i = 0; i < med_idx; i++)
    {
        left->_keys.push_back(std::move(mid->_keys[i]));
    }
    if (!are_leaves)
    {
        for (size_t i = 0; i <= med_idx; i++)
        {
            left->_pointers.push_back(mid->_pointers[i]);
        }
    }

    parent->_keys[mid_child_index - 1] = std::move(mid->_keys[med_idx]);

    mid->_keys.erase(mid->_keys.begin(), mid->_keys.begin() + (med_idx + 1));

    if (!are_leaves)
    {
        mid->_pointers.erase(mid->_pointers.begin(), mid->_pointers.begin() + (med_idx + 1));
    }

    mid->_keys.push_back(parent->_keys[mid_child_index]);
    for (size_t i = 0; i < right->_keys.size(); i++)
    {
        mid->_keys.push_back(right->_keys[i]);
    }
    if (!are_leaves)
    {
        for (size_t i = 0; i < right->_pointers.size(); i++)
        {
            mid->_pointers.push_back(right->_pointers[i]);
        }
    }

    parent->_keys.erase(parent->_keys.begin() + mid_child_index);
    parent->_pointers.erase(parent->_pointers.begin() + (mid_child_index + 1));

    delete_node(right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::ensure_child_has_enough_keys(bstree_node* node, size_t& idx)
{
    if (node->_pointers[idx]->_keys.size() >= minimum_keys_in_node)
        return;

    if (idx > 0 && node->_pointers[idx - 1]->_keys.size() > minimum_keys_in_node)
    {
        borrow_from_left(node, idx);
    }
    else if (idx < node->_pointers.size() - 1 && node->_pointers[idx + 1]->_keys.size() > minimum_keys_in_node)
    {
        borrow_from_right(node, idx);
    }
    else if (idx == 0
        && node->_pointers.size() >= 3
        && node->_pointers[idx + 2]->_keys.size() > minimum_keys_in_node)
    {
        borrow_from_right(node, idx + 1);
        borrow_from_right(node, idx);
    }
    else if (idx == node->_pointers.size() - 1
        && node->_pointers.size() >= 3
        && node->_pointers[idx - 2]->_keys.size() > minimum_keys_in_node)
    {
        borrow_from_left(node, idx - 1);
        borrow_from_left(node, idx);
    }
    else
    {
        if (node->_pointers.size() >= 3)
        {
            if (idx == 0)
                merge_3_children(node, idx + 1);
            else if (idx == node->_pointers.size() - 1)
                merge_3_children(node, idx - 1);
            else
                merge_3_children(node, idx);
        }
        else
        {
            merge_2_children(node, 0);
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::try_erase_from_node(bstree_node* node, const tkey& key_to_erase)
{
    tkey key = key_to_erase;
    size_t idx = find_key_index(node, key);
    if (idx < node->_keys.size() && keys_equal(node->_keys[idx].first, key))
    {
        if (node->_pointers.empty())
        {
            node->_keys.erase(node->_keys.begin() + idx);
            return true;
        }

        tree_data_type left = get_max_key(node->_pointers[idx]);
        node->_keys[idx] = left;
        key = left.first;
    }

    if (node->_pointers.empty())
    {
        return false;
    }

    bool res = try_erase_from_node(node->_pointers[idx], key);

    if (res)
        ensure_child_has_enough_keys(node, idx);
    return res;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator
BS_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    if (_root == nullptr)
        return end();

    if (!try_erase_from_node(_root, key))
        return end();

    --_size;

    if (_root->_keys.empty())
    {
        bstree_node* old_root = _root;
        
        if (_root->_pointers.empty())
        {
            _root = nullptr;
        }
        else
        {
            _root = _root->_pointers[0];
        }

        delete_node(old_root);
    }

    return lower_bound(key);
}
#endif