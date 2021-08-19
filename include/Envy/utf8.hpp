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

#include <iterator>
#include <concepts>
#include <type_traits>
#include <compare>
#include <ostream>

namespace Envy::utf8
{
    using code_unit = u8;

    class code_point final
    {

        u32 cp {};

    public:

        code_point() = default;
        code_point(code_unit value) noexcept;
        code_point(u32 value) noexcept;
        code_point(char value) noexcept;
        code_point(char8_t value) noexcept;
        code_point(const char* value) noexcept;

        [[nodiscard]] u32& get() noexcept;

        [[nodiscard]] explicit operator u32() const noexcept;

        [[nodiscard]] bool operator==(const code_point&) const = default;
        [[nodiscard]] auto operator<=>(const code_point&) const = default;
    };

    std::ostream& operator<<(std::ostream& os, code_point cp);

    [[nodiscard]] i32 code_units_encoded(const code_unit* lead_unit) noexcept(!Envy::debug);
    [[nodiscard]] i32 code_units_required(code_point cp) noexcept;
    [[nodiscard]] i32 count_code_points(const code_unit* buffer) noexcept(!Envy::debug);
    [[nodiscard]] i32 count_code_points(const code_unit* buffer, usize size_bytes) noexcept(!Envy::debug);
    [[nodiscard]] usize byte_len(const code_unit* buffer) noexcept;

    [[nodiscard]] bool is_lead_unit(const code_unit* unit) noexcept;
    [[nodiscard]] bool is_continuation_unit(const code_unit* unit) noexcept;
    [[nodiscard]] bool is_valid_char(const code_unit* lead) noexcept;

    void increment_ptr(const code_unit** lead_ptr) noexcept(!Envy::debug);
    void decrement_ptr(const code_unit** lead_ptr) noexcept(!Envy::debug);
    void increment_ptr(code_unit** lead_ptr) noexcept(!Envy::debug);
    void decrement_ptr(code_unit** lead_ptr) noexcept(!Envy::debug);

    [[nodiscard]] code_point decode(const code_unit* lead) noexcept(!Envy::debug);
    void encode(code_point cp, code_unit* ptr);

    [[nodiscard("consider using 'Envy::utf8::increment_ptr()'")]]
    code_point next_code_point(const code_unit** lead_ptr) noexcept(!Envy::debug);

    [[nodiscard("consider using 'Envy::utf8::decrement_ptr()'")]]
    code_point prev_code_point(const code_unit** lead_ptr) noexcept(!Envy::debug);

    [[nodiscard]] bool is_valid_utf8(const code_unit* buffer) noexcept;

    class iterator final
    {
        const code_unit* ptr;
    public:

        // member types
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = code_point;
        using pointer           = value_type*;
        using reference         = value_type&;

        iterator() noexcept;
        iterator(const code_unit*) noexcept;

        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        iterator& operator++() noexcept(!Envy::debug);
        iterator operator++(int) noexcept(!Envy::debug);

        iterator& operator--() noexcept(!Envy::debug);
        iterator operator--(int) noexcept(!Envy::debug);

        [[nodiscard]] value_type operator*() const noexcept(!Envy::debug);

        [[nodiscard]] bool operator==(const iterator&) const noexcept;

        // iterator non-member funcs

        [[nodiscard]] friend const code_unit* iterator_ptr(const iterator&) noexcept;
        [[nodiscard]] friend usize iterator_distance_bytes(const iterator&, const iterator&) noexcept;

    };

    [[nodiscard]] const code_unit* iterator_ptr(const iterator&) noexcept;
    [[nodiscard]] usize iterator_distance_bytes(const iterator&, const iterator&) noexcept;

    using reverse_iterator = std::reverse_iterator<iterator>;

    // iterator requirments
    static_assert(std::indirectly_readable<iterator>);
    static_assert(std::incrementable<iterator>);
    static_assert(std::input_or_output_iterator<iterator>);
    static_assert(std::forward_iterator<iterator>);
    static_assert(std::bidirectional_iterator<iterator>);

}