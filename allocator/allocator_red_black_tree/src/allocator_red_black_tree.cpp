#include <not_implemented.h>

#include <cstring>
#include "../include/allocator_red_black_tree.h"

namespace
{
    inline char* bytes(void* p) noexcept
    {
        return static_cast<char*>(p);
    }

    inline const char* bytes(const void* p) noexcept
    {
        return static_cast<const char*>(p);
    }

    inline std::pmr::memory_resource*& parent_allocator_ref(void* trusted) {
        return *reinterpret_cast<std::pmr::memory_resource**>(trusted);
    }

    inline allocator_with_fit_mode::fit_mode& fit_mode_ref(void* trusted) noexcept
    {
        return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(
            bytes(trusted) + sizeof(std::pmr::memory_resource*));
    }

    inline const allocator_with_fit_mode::fit_mode& fit_mode_ref(const void* trusted) noexcept
    {
        return *reinterpret_cast<const allocator_with_fit_mode::fit_mode*>(
            bytes(trusted) + sizeof(std::pmr::memory_resource*));
    }

    inline size_t& total_space_ref(void* trusted) noexcept
    {
        return *reinterpret_cast<size_t*>(
            bytes(trusted)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode));
    }

    inline const size_t& total_space_ref(const void* trusted) noexcept
    {
        return *reinterpret_cast<const size_t*>(
            bytes(trusted)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode));
    }

    inline std::mutex& mutex_ref(void* trusted) noexcept
    {
        return *reinterpret_cast<std::mutex*>(
            bytes(trusted)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t));
    }

    inline const std::mutex& mutex_ref(const void* trusted) noexcept
    {
        return *reinterpret_cast<const std::mutex*>(
            bytes(trusted)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t));
    }

    inline void*& root_ref(void* trusted) {
        return *reinterpret_cast<void**>(
            bytes(trusted)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t)
            + sizeof(std::mutex));
    }

    inline void* const& root_ref(const void* trusted) {
        return *reinterpret_cast<void* const*>(
            bytes(trusted)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t)
            + sizeof(std::mutex));
    }

    enum class block_color : unsigned char
    {
        RED, BLACK
    };

    struct block_data
    {
        bool occupied : 4;
        block_color color : 4;
    };

    static constexpr const size_t allocator_metadata_size = sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);
    static constexpr const size_t occupied_block_metadata_size = sizeof(block_data) + 3 * sizeof(void*);
    static constexpr const size_t free_block_metadata_size = sizeof(block_data) + 5 * sizeof(void*);


    inline block_data& block_data_ref(void* block)
    {
        return *reinterpret_cast<block_data*>(block);
    }

    inline block_data* block_data_ptr(void* block)
    {
        return reinterpret_cast<block_data*>(block);
    }

    inline bool is_occupied(void* block)
    {
        return block_data_ptr(block)->occupied;
    }

    inline block_color block_color_of(void* block)
    {
        return block == nullptr ? block_color::BLACK : block_data_ptr(block)->color;
    }


    inline void*& prev_block_ref(void* block) noexcept
    {
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data));
    }

    inline void* const& prev_block_ref(const void* block) noexcept
    {
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data));
    }

    inline void*& next_block_ref(void* block) noexcept
    {
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data) + sizeof(void*));
    }

    inline void* const& next_block_ref(const void* block) noexcept
    {
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data) + sizeof(void*));
    }


    inline void*& parent_block_ref(void* block) noexcept
    {
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data) + 2 * sizeof(void*));
    }

    inline void* const& parent_block_ref(const void* block) noexcept
    {
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data) + 2 * sizeof(void*));
    }


    inline void*& left_child_block_ref(void* block) noexcept
    {
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data) + 3 * sizeof(void*));
    }

    inline void* const& left_child_block_ref(const void* block) noexcept
    {
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data) + 3 * sizeof(void*));
    }


    inline void*& right_child_block_ref(void* block) noexcept
    {
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data) + 4 * sizeof(void*));
    }

    inline void* const& right_child_block_ref(const void* block) noexcept
    {
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data) + 4 * sizeof(void*));
    }


    inline void* left_child_block_ptr(void* block) noexcept
    {
        if (block == nullptr) return nullptr;
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data) + 3 * sizeof(void*));
    }

    inline void* const left_child_block_ptr(const void* block) noexcept
    {
        if (block == nullptr) return nullptr;
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data) + 3 * sizeof(void*));
    }


    inline void* right_child_block_ptr(void* block) noexcept
    {
        if (block == nullptr) return nullptr;
        return *reinterpret_cast<void**>(bytes(block) + sizeof(block_data) + 4 * sizeof(void*));
    }

    inline void* const right_child_block_ptr(const void* block) noexcept
    {
        if (block == nullptr) return nullptr;
        return *reinterpret_cast<void* const*>(bytes(block) + sizeof(block_data) + 4 * sizeof(void*));
    }

    inline char* first_block_ptr(void* trusted) noexcept
    {
        return bytes(trusted) + allocator_metadata_size;
    }

    inline const char* first_block_ptr(const void* trusted) noexcept
    {
        return bytes(trusted) + allocator_metadata_size;
    }

    inline char* memory_end_ptr(void* trusted)
    {
        return first_block_ptr(trusted) + total_space_ref(trusted);
    }

    inline const char* memory_end_ptr(const void* trusted)
    {
        return first_block_ptr(trusted) + total_space_ref(trusted);
    }

    inline char* user_memory_ptr(void* block) {
        return bytes(block) + occupied_block_metadata_size;
    }

    inline const char* user_memory_ptr(const void* block) {
        return bytes(block) + occupied_block_metadata_size;
    }

    inline size_t block_size_of(void* block, void* _trusted)
    {
        void* end = next_block_ref(block);
        if (end == nullptr)
            end = memory_end_ptr(_trusted);

        return bytes(end) - bytes(block);
    }

    inline void reset_free_block_for_insert(void* block)
    {
        block_data_ref(block).occupied = false;
        block_data_ref(block).color = block_color::RED;
        left_child_block_ref(block) = nullptr;
        right_child_block_ref(block) = nullptr;
        parent_block_ref(block) = nullptr;
    }

    inline void reset_free_block(void* block)
    {
        reset_free_block_for_insert(block);
        prev_block_ref(block) = nullptr;
        next_block_ref(block) = nullptr;
    }


    inline void set_root_color(void* block)
    {
        block_data_ref(block).color = block_color::BLACK;
    }

    inline void release(void* trusted)
    {
        if (trusted == nullptr)
            return;

        auto* parent = parent_allocator_ref(trusted);
        size_t bytes_to_free = allocator_metadata_size + total_space_ref(trusted);

        mutex_ref(trusted).~mutex();
        parent->deallocate(trusted, bytes_to_free, alignof(std::max_align_t));
        trusted = nullptr;
    }


    inline void transplant(void* u, void* v, void* trusted)
    {
        if (parent_block_ref(u) == nullptr)
        {
            root_ref(trusted) = v;
        }
        else if (left_child_block_ref(parent_block_ref(u)) == u)
        {
            left_child_block_ref(parent_block_ref(u)) = v;
        }
        else
        {
            right_child_block_ref(parent_block_ref(u)) = v;
        }
        if (v != nullptr)
        {
            parent_block_ref(v) = parent_block_ref(u);
        }
    }

    inline void rotate_left(void* x, void* trusted)
    {
        if (x == nullptr || x == root_ref(trusted) || parent_block_ref(x) == nullptr)
            return;

        void* tmp = left_child_block_ref(x);
        left_child_block_ref(x) = parent_block_ref(x);
        right_child_block_ref(parent_block_ref(x)) = tmp;
        if (tmp != nullptr)
        {
            parent_block_ref(tmp) = parent_block_ref(x);
        }

        transplant(parent_block_ref(x), x, trusted);
        parent_block_ref(left_child_block_ref(x)) = x;
    }

    inline void rotate_right(void* x, void* trusted)
    {
        if (x == nullptr || x == root_ref(trusted) || parent_block_ref(x) == nullptr)
            return;

        void* tmp = right_child_block_ref(x);
        right_child_block_ref(x) = parent_block_ref(x);
        left_child_block_ref(parent_block_ref(x)) = tmp;
        if (tmp != nullptr)
        {
            parent_block_ref(tmp) = parent_block_ref(x);
        }

        transplant(parent_block_ref(x), x, trusted);
        parent_block_ref(right_child_block_ref(x)) = x;
    }

    inline void* grandparent(void* node)
    {
        void* parent = parent_block_ref(node);
        if (parent && parent_block_ref(parent))
            return parent_block_ref(parent);
        return nullptr;
    }

    inline void* uncle(void* node)
    {
        void* gp = grandparent(node);
        if (gp == nullptr)
            return nullptr;
        if (parent_block_ref(node) == left_child_block_ref(gp))
            return right_child_block_ref(gp);
        return left_child_block_ref(gp);
    }

    inline bool is_right_child(void* node)
    {
        if (node == nullptr) return false;
        if (parent_block_ref(node) == nullptr) return false;
        if (right_child_block_ref(parent_block_ref(node)) == node) return true;
        return false;
    }

    inline bool is_left_child(void* node)
    {
        if (node == nullptr) return false;
        if (parent_block_ref(node) == nullptr) return false;
        if (left_child_block_ref(parent_block_ref(node)) == node) return true;
        return false;
    }

    inline void insert_case_1(void* node, void* trusted);
    inline void insert_case_2(void* node, void* trusted);
    inline void insert_case_3(void* node, void* trusted);
    inline void insert_case_4(void* node, void* trusted);
    inline void insert_case_5(void* node, void* trusted);

    inline void on_node_added(void* block, void* trusted)
    {
        insert_case_1(block, trusted);
    }

    inline void insert_case_1(void* node, void* trusted)
    {
        if (parent_block_ref(node) == nullptr)
            block_data_ref(node).color = block_color::BLACK;
        else
            insert_case_2(node, trusted);
    }

    inline void insert_case_2(void* node, void* trusted)
    {
        if (block_data_ref(parent_block_ref(node)).color == block_color::RED)
        {
            insert_case_3(node, trusted);
        }
    }

    inline void insert_case_3(void* node, void* trusted)
    {
        void* unc = uncle(node);
        if (unc != nullptr && block_data_ref(unc).color == block_color::RED)
        {
            block_data_ref(parent_block_ref(node)).color = block_color::BLACK;
            block_data_ref(unc).color = block_color::BLACK;
            void* gp = grandparent(node);
            block_data_ref(gp).color = block_color::RED;
            insert_case_1(gp, trusted);
        }
        else
        {
            insert_case_4(node, trusted);
        }
    }

    inline void insert_case_4(void* node, void* trusted)
    {
        void* gp = grandparent(node);
        if (is_right_child(node) && is_left_child(parent_block_ref(node)))
        {
            rotate_left(node, trusted);
            node = left_child_block_ref(node);
        }
        else if (is_left_child(node) && is_right_child(parent_block_ref(node)))
        {
            rotate_right(node, trusted);
            node = right_child_block_ref(node);
        }
        insert_case_5(node, trusted);
    }

    inline void insert_case_5(void* node, void* trusted)
    {
        void* parent = parent_block_ref(node);
        void* gp = grandparent(node);

        block_data_ref(parent).color = block_color::BLACK;
        block_data_ref(gp).color = block_color::RED;
        if (is_left_child(node) && is_left_child(parent))
        {
            rotate_right(parent, trusted);
        }
        else
        {
            rotate_left(parent, trusted);
        }
    }

    inline void add_node(void* block, void* trusted)
    {
        if (root_ref(trusted) == nullptr)
        {
            root_ref(trusted) = block;
            parent_block_ref(block) = nullptr;
            left_child_block_ref(block) = nullptr;
            right_child_block_ref(block) = nullptr;
            on_node_added(block, trusted);
            return;
        }

        void* current = root_ref(trusted);
        bool node_added = false;

        while (current != nullptr)
        {
            if (block_size_of(block, trusted) <= block_size_of(current, trusted))
            {
                if (left_child_block_ref(current) != nullptr)
                {
                    current = left_child_block_ref(current);
                }
                else
                {
                    left_child_block_ref(current) = block;
                    node_added = true;
                    break;
                }
            }
            else
            {
                if (right_child_block_ref(current) != nullptr)
                {
                    current = right_child_block_ref(current);
                }
                else
                {
                    right_child_block_ref(current) = block;
                    node_added = true;
                    break;
                }
            }
        }

        if (node_added)
        {
            parent_block_ref(block) = current;
            left_child_block_ref(block) = nullptr;
            right_child_block_ref(block) = nullptr;
            on_node_added(block, trusted);
        }
    }

    inline void remove_case_1(void* parent, void* child, void* trusted);
    inline void remove_case_2(void* parent, void* child, void* trusted);
    inline void remove_case_3(void* parent, void* child, void* trusted);
    inline void remove_case_4(void* parent, void* child, void* trusted);
    inline void remove_case_5(void* parent, void* child, void* trusted);
    inline void remove_case_6(void* parent, void* child, void* trusted);

    inline void on_node_removed(void* parent, void* child, block_color deleted_color, void* trusted)
    {
        if (deleted_color == block_color::BLACK)
        {
            if (child != nullptr && block_color_of(child) == block_color::RED)
            {
                block_data_ref(child).color = block_color::BLACK;
            }
            else
            {
                remove_case_1(parent, child, trusted);
            }
        }
    }

    inline void remove_case_1(void* parent, void* child, void* trusted)
    {
        if (parent != nullptr)
        {
            remove_case_2(parent, child, trusted);
        }
    }

    inline void* sibling(void* parent, void* child)
    {
        if (left_child_block_ref(parent) == child)
            return right_child_block_ref(parent);
        return left_child_block_ref(parent);
    }

    inline void remove_case_2(void* parent, void* child, void* trusted)
    {
        void* sib = sibling(parent, child);

        if (sib != nullptr && block_color_of(sib) == block_color::RED)
        {
            block_data_ref(parent).color = block_color::RED;
            block_data_ref(sib).color = block_color::BLACK;
            if (is_left_child(sib))
            {
                rotate_right(sib, trusted);
            }
            else
            {
                rotate_left(sib, trusted);
            }
        }
        remove_case_3(parent, child, trusted);
    }

    inline void remove_case_3(void* parent, void* child, void* trusted)
    {
        void* sib = sibling(parent, child);
        if ((block_color_of(parent) == block_color::BLACK)
            && (block_color_of(sib) == block_color::BLACK)
            && (left_child_block_ptr(sib) == nullptr || block_color_of(left_child_block_ref(sib)) == block_color::BLACK)
            && (right_child_block_ptr(sib) == nullptr || block_color_of(right_child_block_ref(sib)) == block_color::BLACK))
        {
            block_data_ref(sib).color = block_color::RED;
            remove_case_1(parent_block_ref(parent), parent, trusted);
        }
        else
        {
            remove_case_4(parent, child, trusted);
        }
    }

    inline void remove_case_4(void* parent, void* child, void* trusted)
    {
        void* sib = sibling(parent, child);
        if ((block_color_of(parent) == block_color::RED)
            && (block_color_of(sib) == block_color::BLACK)
            && (left_child_block_ptr(sib) == nullptr || block_color_of(left_child_block_ref(sib)) == block_color::BLACK)
            && (right_child_block_ptr(sib) == nullptr || block_color_of(right_child_block_ref(sib)) == block_color::BLACK))
        {
            block_data_ref(sib).color = block_color::RED;
            block_data_ref(parent).color = block_color::BLACK;
        }
        else
        {
            remove_case_5(parent, child, trusted);
        }
    }

    inline void remove_case_5(void* parent, void* child, void* trusted)
    {
        void* sib = sibling(parent, child);
        if (block_color_of(sib) == block_color::BLACK)
        {
            if (is_right_child(sib)
                && (left_child_block_ptr(sib) != nullptr && block_color_of(left_child_block_ref(sib)) == block_color::RED)
                && (right_child_block_ptr(sib) == nullptr || block_color_of(right_child_block_ref(sib)) == block_color::BLACK))
            {
                block_data_ref(sib).color = block_color::RED;
                block_data_ref(left_child_block_ref(sib)).color = block_color::BLACK;
                rotate_right(left_child_block_ref(sib), trusted);
            }
            else if (is_left_child(sib)
                && (left_child_block_ptr(sib) == nullptr || block_color_of(left_child_block_ref(sib)) == block_color::BLACK)
                && (right_child_block_ptr(sib) != nullptr && block_color_of(right_child_block_ref(sib)) == block_color::RED))
            {
                block_data_ref(sib).color = block_color::RED;
                block_data_ref(right_child_block_ref(sib)).color = block_color::BLACK;
                rotate_left(right_child_block_ref(sib), trusted);
            }
        }
        remove_case_6(parent, child, trusted);
    }

    inline void remove_case_6(void* parent, void* child, void* trusted)
    {
        void* sib = sibling(parent, child);
        block_data_ref(sib).color = block_color_of(parent);
        block_data_ref(parent).color = block_color::BLACK;

        if (is_right_child(sib))
        {
            block_data_ref(right_child_block_ref(sib)).color = block_color::BLACK;
            rotate_left(sib, trusted);
        }
        else
        {
            block_data_ref(left_child_block_ref(sib)).color = block_color::BLACK;
            rotate_right(sib, trusted);
        }
    }


    inline void* find_rightmost(void* node)
    {
        void* prev = node;
        void* cur = node;
        while (cur != nullptr)
        {
            prev = cur;
            cur = right_child_block_ref(cur);
        }
        return prev;
    }

    inline void remove_node(void* block, void* trusted)
    {
        if (left_child_block_ref(block) == nullptr && right_child_block_ref(block) == nullptr)
        {
            transplant(block, nullptr, trusted);
            on_node_removed(parent_block_ref(block), nullptr, block_color_of(block), trusted);
            return;
        }

        if (left_child_block_ref(block) == nullptr)
        {
            transplant(block, right_child_block_ref(block), trusted);
            on_node_removed(parent_block_ref(block), right_child_block_ref(block), block_color_of(block), trusted);
            return;
        }

        if (right_child_block_ref(block) == nullptr)
        {
            transplant(block, left_child_block_ref(block), trusted);
            on_node_removed(parent_block_ref(block), left_child_block_ref(block), block_color_of(block), trusted);
            return;
        }

        void* replacement = find_rightmost(left_child_block_ref(block));

        void* replacement_parent = parent_block_ref(replacement) == block ? replacement : parent_block_ref(replacement);
        void* replacement_for_replacement = left_child_block_ref(replacement);
        block_color deleted_color = block_color_of(replacement);

        transplant(replacement, replacement_for_replacement, trusted);

        right_child_block_ref(replacement) = right_child_block_ref(block);
        if (right_child_block_ref(replacement) != nullptr)
            parent_block_ref(right_child_block_ref(replacement)) = replacement;

        left_child_block_ref(replacement) = left_child_block_ref(block);
        if (left_child_block_ref(replacement) != nullptr)
            parent_block_ref(left_child_block_ref(replacement)) = replacement;

        block_data_ref(replacement).color = block_data_ref(block).color;
        transplant(block, replacement, trusted);

        on_node_removed(replacement_parent, replacement_for_replacement, deleted_color, trusted);
    }
}

allocator_red_black_tree::~allocator_red_black_tree()
{
    release(_trusted_memory);
}

allocator_red_black_tree::allocator_red_black_tree(
    allocator_red_black_tree &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_red_black_tree &allocator_red_black_tree::operator=(
    allocator_red_black_tree &&other) noexcept
{
    if (this == &other)
        return *this;

    release(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}

allocator_red_black_tree::allocator_red_black_tree(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    auto* parent = parent_allocator == nullptr
        ? std::pmr::get_default_resource()
        : parent_allocator;

    _trusted_memory = parent->allocate(
        allocator_metadata_size + space_size,
        alignof(std::max_align_t));

    parent_allocator_ref(_trusted_memory) = parent;
    fit_mode_ref(_trusted_memory) = allocate_fit_mode;
    total_space_ref(_trusted_memory) = space_size;
    new(&mutex_ref(_trusted_memory)) std::mutex();

    if (space_size >= free_block_metadata_size + 1)
    {
        void* first_block = first_block_ptr(_trusted_memory);
        root_ref(_trusted_memory) = first_block;
        reset_free_block(first_block);
        set_root_color(first_block);
    }
    else
    {
        root_ref(_trusted_memory) = nullptr;
    }
}

allocator_red_black_tree::allocator_red_black_tree(const allocator_red_black_tree &other)
    : _trusted_memory(nullptr)
{
    if (other._trusted_memory == nullptr)
        return;

    auto* parent = parent_allocator_ref(other._trusted_memory);
    size_t space = total_space_ref(other._trusted_memory);

    _trusted_memory = parent->allocate(
        allocator_metadata_size + space,
        alignof(std::max_align_t));

    parent_allocator_ref(_trusted_memory) = parent;
    fit_mode_ref(_trusted_memory) = fit_mode_ref(other._trusted_memory);
    total_space_ref(_trusted_memory) = space;
    new(&mutex_ref(_trusted_memory)) std::mutex();

    std::memcpy(
        first_block_ptr(_trusted_memory),
        first_block_ptr(other._trusted_memory),
        space);

    root_ref(_trusted_memory) = root_ref(other._trusted_memory) != nullptr
                                ? bytes(root_ref(other._trusted_memory)) - bytes(other._trusted_memory) + bytes(_trusted_memory)
                                : nullptr;

    void* old_cur = root_ref(other._trusted_memory) != nullptr
                    ? first_block_ptr(other._trusted_memory)
                    : nullptr;
    void* new_prev = nullptr;

    while (old_cur != nullptr)
    {
        ptrdiff_t offset = bytes(_trusted_memory) - bytes(other._trusted_memory);
        void* new_cur = bytes(old_cur) + offset;
        prev_block_ref(new_cur) = new_prev;
        if (new_prev != nullptr) next_block_ref(new_prev) = new_cur;
        if (is_occupied(old_cur))
        {
            parent_block_ref(new_cur) = _trusted_memory;
        }
        else
        {
            if (left_child_block_ref(old_cur) != nullptr)
                left_child_block_ref(new_cur) = bytes(left_child_block_ref(old_cur)) + offset;
            if (right_child_block_ref(old_cur) != nullptr)
                right_child_block_ref(new_cur) = bytes(right_child_block_ref(old_cur)) + offset;
            if (parent_block_ref(old_cur) != nullptr)
                parent_block_ref(new_cur) = bytes(parent_block_ref(old_cur)) + offset;
        }

        new_prev = new_cur;
        old_cur = next_block_ref(old_cur);
    }

    if (new_prev != nullptr)
        next_block_ref(new_prev) = nullptr;
}

allocator_red_black_tree &allocator_red_black_tree::operator=(const allocator_red_black_tree &other)
{
    if (this == &other)
        return *this;

    *this = allocator_red_black_tree(other);
    return *this;
}

bool allocator_red_black_tree::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == dynamic_cast<const allocator_red_black_tree*>(&other);
}

[[nodiscard]] void *allocator_red_black_tree::do_allocate_sm(
    size_t size)
{
    if (_trusted_memory == nullptr)
        return nullptr;

    std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));

    void* selected = nullptr;

    void* prev = nullptr;
    void* cur = root_ref(_trusted_memory);

    auto mode = fit_mode_ref(_trusted_memory);

    while (cur != nullptr)
    {
        size_t cur_size = block_size_of(cur, _trusted_memory);

        if (cur_size - free_block_metadata_size >= size)
        {
            if (mode == fit_mode::first_fit)
            {
                selected = cur;
                break;
            }
            else if (mode == fit_mode::the_best_fit 
                    && (selected == nullptr || cur_size < block_size_of(selected, _trusted_memory)))
            {
                selected = cur;
                cur = left_child_block_ref(cur);
            }
            else if (mode == fit_mode::the_worst_fit 
                    && (selected == nullptr || cur_size > block_size_of(selected, _trusted_memory)))
            {
                selected = cur;
                cur = right_child_block_ref(cur);
            }
        }
        else
        {
            cur = right_child_block_ref(cur);
        }
    }

    if (selected == nullptr)
        throw std::bad_alloc();

    size_t selected_size = block_size_of(selected, _trusted_memory);
    size_t remainder = selected_size - free_block_metadata_size - size;
    if (remainder >= free_block_metadata_size + 1)
    {
        //size_t selected_new_size = std::max(occupied_block_metadata_size + size,
        //                                    free_block_metadata_size + 1);
        size_t selected_new_size = free_block_metadata_size + size;
        void* new_free_block = bytes(selected) + selected_new_size;
        reset_free_block(new_free_block);

        prev_block_ref(new_free_block) = selected;
        next_block_ref(new_free_block) = next_block_ref(selected);
        block_data_ref(new_free_block).occupied = false;
        next_block_ref(selected) = new_free_block;
        if (next_block_ref(new_free_block) != nullptr)
            prev_block_ref(next_block_ref(new_free_block)) = new_free_block;

        // REMOVE OLD BLOCK
        remove_node(selected, _trusted_memory);
        // INSERT NEW BLOCK
        add_node(new_free_block, _trusted_memory);
    }
    else
    {
        // REMOVE OLD BLOCK
        remove_node(selected, _trusted_memory);
    }
    block_data_ref(selected).occupied = true;
    parent_block_ref(selected) = _trusted_memory;

    return user_memory_ptr(selected);
}

void allocator_red_black_tree::do_deallocate_sm(
    void *at)
{
    if (_trusted_memory == nullptr || at == nullptr)
        return;

    std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));

    char* p = bytes(at);
    char* begin_user_area = first_block_ptr(_trusted_memory) + occupied_block_metadata_size;
    char* end = memory_end_ptr(_trusted_memory);

    if (p < begin_user_area || p >= end)
        throw std::logic_error("pointer does not belong to this allocator");

    void* block = p - occupied_block_metadata_size;

    if (parent_block_ref(block) != _trusted_memory)
        throw std::logic_error("pointer does not belong to this allocator");

    if (!is_occupied(block))
        throw std::logic_error("block is already free");

    block_data_ref(block).occupied = false;
    void* prev_block = prev_block_ref(block);
    void* next_block = next_block_ref(block);
    if (prev_block != nullptr && !is_occupied(prev_block))
    {
        // REMOVE PREV BLOCK
        remove_node(prev_block, _trusted_memory);

        next_block_ref(prev_block) = next_block;
        block = prev_block;
        if (next_block != nullptr)
            prev_block_ref(next_block) = block;

    }

    if (next_block != nullptr && !is_occupied(next_block))
    {
        // REMOVE NEXT BLOCK
        remove_node(next_block, _trusted_memory);
        next_block = next_block_ref(next_block);
        next_block_ref(block) = next_block;
        if (next_block != nullptr)
            prev_block_ref(next_block) = block;
    }

    reset_free_block_for_insert(block);
    // ADD NEW CUR BLOCK
    add_node(block, _trusted_memory);
}

void allocator_red_black_tree::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
    fit_mode_ref(_trusted_memory) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info() const
{
    std::lock_guard<std::mutex> lock(mutex_ref(_trusted_memory));
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> vec;

    for (auto it = begin(); it != end(); it++)
    {
        vec.push_back({ it.size(), it.occupied() });
    }

    return vec;
}


allocator_red_black_tree::rb_iterator allocator_red_black_tree::begin() const noexcept
{
    return rb_iterator(_trusted_memory);
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::end() const noexcept
{
    return rb_iterator();
}


bool allocator_red_black_tree::rb_iterator::operator==(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return _block_ptr == other._block_ptr;
}

bool allocator_red_black_tree::rb_iterator::operator!=(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_red_black_tree::rb_iterator &allocator_red_black_tree::rb_iterator::operator++() & noexcept
{
    if (_block_ptr != nullptr)
        _block_ptr = next_block_ref(_block_ptr);
    return *this;
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::rb_iterator::operator++(int n)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

size_t allocator_red_black_tree::rb_iterator::size() const noexcept
{
    if (_block_ptr == nullptr) return 0;

    size_t size = block_size_of(_block_ptr, _trusted);
    if (is_occupied(_block_ptr)) return size - occupied_block_metadata_size;

    return size - free_block_metadata_size;
}

void *allocator_red_black_tree::rb_iterator::operator*() const noexcept
{
    return _block_ptr;
}

allocator_red_black_tree::rb_iterator::rb_iterator()
    : _trusted(nullptr), _block_ptr(nullptr)
{
}

allocator_red_black_tree::rb_iterator::rb_iterator(void *trusted)
    : _trusted(nullptr), _block_ptr(nullptr)
{
    if (trusted == nullptr)
        return;

    _trusted = trusted;
    if (root_ref(trusted) != nullptr)
        _block_ptr = first_block_ptr(trusted);
}

bool allocator_red_black_tree::rb_iterator::occupied() const noexcept
{
    if (_block_ptr == nullptr) return false;

    return is_occupied(_block_ptr);
}
