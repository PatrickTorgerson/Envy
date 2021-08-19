///////////////////////////////////////////////////////////////////////////////////////
//
//    Envy Game Engine
//    https://github.com/PatrickTorgerson/Envy
//
//    Copyright (c) 2021 Patrick Torgerson
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.hpp"

#include <concepts>
#include <bit>
#include <type_traits>
#include <cstddef>
#include <ranges>

namespace Envy
{

    template <typename T>
    concept trivially_copyable =
        std::copyable<T> &&
        std::is_trivially_copyable_v<T>;

    template <typename T>
    concept buffer_safe =
        trivially_copyable<T>
        || (std::is_array_v<T> && trivially_copyable<std::remove_all_extents_t<T>>);


    template <buffer_safe T>
    [[nodiscard]] constexpr auto byte_pointer(T* ptr) noexcept
    {
        auto void_ptr = static_cast<void*>(ptr);
        return static_cast<u8*>(void_ptr);
    }


    template <buffer_safe T>
    [[nodiscard]] auto byte_pointer(const T* ptr) noexcept
    {
        auto void_ptr = static_cast<const void*>(ptr);
        return static_cast<const u8*>(void_ptr);
    }


    template <typename T>
        requires buffer_safe<std::remove_cvref_t<T>>
    [[nodiscard]] auto byte_addressof(T&& obj) noexcept
    {
        return byte_pointer(std::addressof(obj));
    }


    template <typename T>
    concept trivial_range =
        std::ranges::contiguous_range<T> &&
        std::ranges::sized_range<T> &&
        // The range's data type should satisfy buffer_safe.
        buffer_safe<std::ranges::range_value_t<T>>;


    template <typename T>
    concept mutable_trivial_range =
        trivial_range<T> &&
        requires (std::ranges::range_reference_t<T> ref)
        {
            { byte_addressof(ref) } -> std::same_as<u8*>;
        };


    template <trivial_range T>
    [[nodiscard]] usize range_size_bytes(const T& c)
    {
        return std::ranges::size(c) * sizeof(std::ranges::range_value_t<decltype(c)>);
    }


    class const_buffer final
    {
        const u8* d {nullptr};
        usize s {0u};
    public:

        const_buffer() = default;
        const_buffer(const const_buffer&) = default;
        const_buffer& operator=(const const_buffer&) = default;

        template <buffer_safe T>
        const_buffer(const T* d, usize s) : d{byte_pointer(d)} , s{s*sizeof(T)} {}

        template <trivial_range T>
        explicit const_buffer(T&& c) :
            const_buffer( std::ranges::data(c) , std::ranges::size(c) )
        { }

        [[nodiscard]] const u8* data() const noexcept;
        [[nodiscard]] usize size() const noexcept;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] operator bool() const noexcept;

        [[nodiscard]] u8 operator[](usize i) const noexcept;
        [[nodiscard]] u8 operator*() const noexcept;

        const_buffer& operator+=(usize offset) noexcept;

        void remove_prefix(usize count);
        void remove_suffix(usize count);

        [[nodiscard]] u8 first() const noexcept;
        [[nodiscard]] u8 last() const noexcept;

        [[nodiscard]] const_buffer first(usize count) const;
        [[nodiscard]] const_buffer last(usize count) const;
        [[nodiscard]] const_buffer sub(usize pos, usize size) const;

    };


    class mutable_buffer final
    {
        u8* d {nullptr};
        usize s {0u};
    public:

        mutable_buffer() = default;
        mutable_buffer(const mutable_buffer&) = default;
        mutable_buffer& operator=(const mutable_buffer&) = default;

        template <buffer_safe T>
        mutable_buffer(T* d, usize s) : d{byte_pointer(d)} , s{s*sizeof(T)} {}

        template <mutable_trivial_range T>
        explicit mutable_buffer(T&& c) :
            mutable_buffer( std::ranges::data(c) , std::ranges::size(c) )
        { }

        [[nodiscard]] u8* data() noexcept;
        [[nodiscard]] const u8* data() const noexcept;
        [[nodiscard]] usize size() const noexcept;

        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] operator bool() const noexcept;
        [[nodiscard]] operator const_buffer() const noexcept;

        [[nodiscard]] u8 operator[](usize i) const noexcept;
        [[nodiscard]] u8 operator*() const noexcept;

        [[nodiscard]] u8& operator[](usize i) noexcept;
        [[nodiscard]] u8& operator*() noexcept;

        mutable_buffer& operator+=(usize offset) noexcept;

        void remove_prefix(usize count);
        void remove_suffix(usize count);

        [[nodiscard]] u8 first() const noexcept;
        [[nodiscard]] u8 last() const noexcept;

        [[nodiscard]] mutable_buffer first(usize count) const;
        [[nodiscard]] mutable_buffer last(usize count) const;
        [[nodiscard]] mutable_buffer sub(usize pos, usize size) const;
    };

}