// Copyright (C) 2015-2016 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include "memory_arena.hpp"

#include <new>

#include "detail/align.hpp"

using namespace foonathan::memory;
using namespace detail;

const std::size_t memory_block_stack::node::div_alignment =
    sizeof(memory_block_stack::node) / max_alignment;
const std::size_t memory_block_stack::node::mod_offset =
    sizeof(memory_block_stack::node) % max_alignment != 0u;
const std::size_t memory_block_stack::node::offset = (div_alignment + mod_offset) * max_alignment;

const std::size_t memory_block_stack::implementation_offset = memory_block_stack::node::offset;

void memory_block_stack::push(allocated_mb block) FOONATHAN_NOEXCEPT
{
    FOONATHAN_MEMORY_ASSERT(is_aligned(block.memory, max_alignment));
    auto next = ::new (block.memory) node(head_, block.size - node::offset);
    head_     = next;
}

memory_block_stack::allocated_mb memory_block_stack::pop() FOONATHAN_NOEXCEPT
{
    FOONATHAN_MEMORY_ASSERT(head_);
    auto to_pop = head_;
    head_       = head_->prev;
    return {to_pop, to_pop->usable_size + node::offset};
}

void memory_block_stack::steal_top(memory_block_stack& other) FOONATHAN_NOEXCEPT
{
    FOONATHAN_MEMORY_ASSERT(other.head_);
    auto to_steal = other.head_;
    other.head_   = other.head_->prev;

    to_steal->prev = head_;
    head_          = to_steal;
}

bool memory_block_stack::owns(const void* ptr) const FOONATHAN_NOEXCEPT
{
    auto address = static_cast<const char*>(ptr);
    for (auto cur = head_; cur; cur = cur->prev)
    {
        auto mem = static_cast<char*>(static_cast<void*>(cur));
        if (address >= mem && address < mem + cur->usable_size)
            return true;
    }
    return false;
}

std::size_t memory_block_stack::size() const FOONATHAN_NOEXCEPT
{
    std::size_t res = 0u;
    for (auto cur = head_; cur; cur = cur->prev)
        ++res;
    return res;
}

#if FOONATHAN_MEMORY_EXTERN_TEMPLATE
template class foonathan::memory::memory_arena<static_block_allocator, true>;
template class foonathan::memory::memory_arena<static_block_allocator, false>;
template class foonathan::memory::memory_arena<virtual_block_allocator, true>;
template class foonathan::memory::memory_arena<virtual_block_allocator, false>;

template class foonathan::memory::growing_block_allocator<>;
template class foonathan::memory::memory_arena<growing_block_allocator<>, true>;
template class foonathan::memory::memory_arena<growing_block_allocator<>, false>;

template class foonathan::memory::fixed_block_allocator<>;
template class foonathan::memory::memory_arena<fixed_block_allocator<>, true>;
template class foonathan::memory::memory_arena<fixed_block_allocator<>, false>;
#endif