#include <not_implemented.h>
#include "../include/allocator_global_heap.h"

#include <new>
#include <mutex>
#include <cstddef>

allocator_global_heap::allocator_global_heap() = default;

[[nodiscard]] void* allocator_global_heap::do_allocate_sm(size_t size)
{
    if (size == 0)
        return nullptr;

    auto* raw_block = static_cast<std::byte*>(::operator new(size_t_size + size));

    *reinterpret_cast<size_t*>(raw_block) = size;

    return raw_block + size_t_size;
}

void allocator_global_heap::do_deallocate_sm(void* at)
{
    if (at == nullptr) return;

    std::byte* user_ptr = static_cast<std::byte*>(at);
    std::byte* raw_block = user_ptr - size_t_size;

    ::operator delete(raw_block);
}

allocator_global_heap::~allocator_global_heap() = default;

allocator_global_heap::allocator_global_heap(const allocator_global_heap& other) = default;

allocator_global_heap& allocator_global_heap::operator=(const allocator_global_heap& other) = default;

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    return dynamic_cast<const allocator_global_heap*>(&other) == this;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap&& other) noexcept = default;
allocator_global_heap& allocator_global_heap::operator=(allocator_global_heap&& other) noexcept = default;