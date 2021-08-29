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

/********************************************************************************
 * \file utf8.hpp
 * \brief UTF-8 Utilities
 ********************************************************************************/

#pragma once

#include "common.hpp"

#include <iterator>
#include <concepts>
#include <type_traits>
#include <compare>
#include <ostream>

namespace Envy::utf8
{
    /********************************************************************************
     * \brief Type alias for a UTF-8 code unit
     * \see Envy::utf8::code_point
     ********************************************************************************/
    using code_unit = u8;

    /********************************************************************************
     * \brief Type representing a UTF-8 code point
     *
     ********************************************************************************/
    class code_point final
    {

        u32 cp {}; ///< The code point value

    public:

        /********************************************************************************
         * \brief Default constructs a code point to the null-terminator
         *
         ********************************************************************************/
        code_point() = default;

        /********************************************************************************
         * \brief Constructs a code_point from a code_unit
         *
         * \param [in] value code_unit, should be an ascii code point
         ********************************************************************************/
        code_point(code_unit value) noexcept;

        /********************************************************************************
         * \brief Constructs a code point from a u32 integer
         *
         * \param [in] value code point
         ********************************************************************************/
        code_point(u32 value) noexcept;

        /********************************************************************************
         * \brief Constructs a code point from an ascii character
         *
         * \param [in] value ascii character
         ********************************************************************************/
        code_point(char value) noexcept;

        /********************************************************************************
         * \brief Constructs a code point from a char8_t
         *
         * \param [in] value code_unit, should be an ascii code point
         ********************************************************************************/
        code_point(char8_t value) noexcept;

        /********************************************************************************
         * \brief Constructs a code point from a UTF-8 sequence
         *
         * \param [in] value should be a valid UTF-8 encoded unicode code point
         ********************************************************************************/
        code_point(const char* value) noexcept;

        /********************************************************************************
         * \brief Returns the code point integer
         *
         * \return u32& code point refference
         ********************************************************************************/
        [[nodiscard]] u32& get() noexcept;

        /********************************************************************************
         * \brief Returns the code point integer
         *
         * \return u32 code point
         ********************************************************************************/
        [[nodiscard]] explicit operator u32() const noexcept;

        /********************************************************************************
         * \brief Equality comparison
         *
         * \return true if the code points are equal
         * \return false if the code points are not equal
         ********************************************************************************/
        [[nodiscard]] bool operator==(const code_point&) const = default;

        /********************************************************************************
         * \brief Comparison
         ********************************************************************************/
        [[nodiscard]] auto operator<=>(const code_point&) const = default;
    };

    /********************************************************************************
     * \brief Output stream inserttion for Envy::code_point
     *
     * \param [in] os output stream
     * \param [in] cp code point
     * \return std::ostream& output stream
     ********************************************************************************/
    std::ostream& operator<<(std::ostream& os, code_point cp);

    /********************************************************************************
     * \brief Returns the number of code units a code point is encoded in
     *
     * \param [in] lead_unit Pointer to the first code unit of a UTF-8 encoded code point
     * \return i32 number of code units the code point is encoded in, will be 1 to 4
     ********************************************************************************/
    [[nodiscard]] i32 code_units_encoded(const code_unit* lead_unit) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Returns the number of code units requred to encode a unicode code unit in UTF-8
     *
     * \param [in] cp code point
     * \return i32 Number of code units requred to encode code point
     ********************************************************************************/
    [[nodiscard]] i32 code_units_required(code_point cp) noexcept;

    /********************************************************************************
     * \brief Return the size of a UTF-8 string in unicode code points
     *
     * \param [in] buffer A valid null-terminated UTF-8 string
     * \return i32 Size in code points
     ********************************************************************************/
    [[nodiscard]] i32 count_code_points(const code_unit* buffer) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Return the size of a UTF-8 string in unicode code points
     *
     * \param [in] buffer A valid UTF-8 string
     * \param [in] size_bytes Size of buffer in bytes
     * \return i32 Size in code points
     ********************************************************************************/
    [[nodiscard]] i32 count_code_points(const code_unit* buffer, usize size_bytes) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Return the size of a UTF-8 string in bytes
     *
     * \param [in] buffer A null-terminated UTF-8 string
     * \return usize Size in bytes
     ********************************************************************************/
    [[nodiscard]] usize size_bytes(const code_unit* buffer) noexcept;

    /********************************************************************************
     * \brief Return the size of a string in bytes
     *
     * \param [in] buffer A null-terminated string
     * \return usize Size in bytes
     ********************************************************************************/
    [[nodiscard]] usize size_bytes(const char* buffer) noexcept;

    /********************************************************************************
     * \brief Returns if a code unit represents the first code unit in a UTF-8 encoded code point
     *
     * \param [in] unit pointer to a code unit
     * \return true, *unit* is a lead unit
     * \return false, *unit* is a continuation unit, or invalid
     ********************************************************************************/
    [[nodiscard]] bool is_lead_unit(const code_unit* unit) noexcept;

    /********************************************************************************
     * \brief Returns if a code unit represents a continuation unit in a UTF-8 encoded code point
     *
     * \param [in] unit pointer to a code unit
     * \return true, *unit* is a continuation unit
     * \return false, *unit* is a lead unit, or invalid
     ********************************************************************************/
    [[nodiscard]] bool is_continuation_unit(const code_unit* unit) noexcept;

    /********************************************************************************
     * \brief Returns if a sequnce of code units represents a unicode cope point encoded in UTF-8
     *
     * \param [in] lead Pointer to the first code unit in a sequnce of code units
     * \return true, *lead* points to a valid UTF-8 encoded code point
     * \return false, *lead* points to a valid UTF-8 encoded code point
     ********************************************************************************/
    [[nodiscard]] bool is_valid_char(const code_unit* lead) noexcept;

    /********************************************************************************
     * \brief Increments a pointer to the next code point in a UTF-8 string
     *
     * \param [in] lead_ptr Pointer to a lead code unit in a UTF-8 string
     ********************************************************************************/
    void increment_ptr(const code_unit** lead_ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Decrements a pointer to the prev code point in a UTF-8 string
     *
     * \param [in] lead_ptr Pointer to a lead code unit in a UTF-8 string
     ********************************************************************************/
    void decrement_ptr(const code_unit** lead_ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Increments a pointer to the next code point in a UTF-8 string
     *
     * \param [in] lead_ptr Pointer to a lead code unit in a UTF-8 string
     ********************************************************************************/
    void increment_ptr(code_unit** lead_ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Decrements a pointer to the prev code point in a UTF-8 string
     *
     * \param [in] lead_ptr Pointer to a lead code unit in a UTF-8 string
     ********************************************************************************/
    void decrement_ptr(code_unit** lead_ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Decodes a unicode code point from a UTF-8 string
     *
     * \param [in] lead  Pointer to a lead code unit in a UTF-8 string
     * \return code_point Decoded code point
     ********************************************************************************/
    [[nodiscard]] code_point decode(const code_unit* lead) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Encodes a unicode code point to a UTF-8 string
     *
     * It is the programmers resposibility to insure ther is enough room in the
     * buffer for the code point.
     *
     * \param [in] cp code point to encode
     * \param [in] ptr poiter to write encoded code point to
     ********************************************************************************/
    void encode(code_point cp, code_unit* ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Decodes the current code point and increments to the next
     *
     * \param [in] lead_ptr Pointer to a lead code unit in a UTF-8 string
     * \return code_point Decoded code point
     ********************************************************************************/
    [[nodiscard("consider using 'Envy::utf8::increment_ptr()'")]]
    code_point next_code_point(const code_unit** lead_ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Decodes the current code point and decrements to the previous
     *
     * \param [in] lead_ptr Pointer to a lead code unit in a UTF-8 string
     * \return code_point Decoded code point
     ********************************************************************************/
    [[nodiscard("consider using 'Envy::utf8::decrement_ptr()'")]]
    code_point prev_code_point(const code_unit** lead_ptr) noexcept(!Envy::debug);

    /********************************************************************************
     * \brief Checks if a string is a valid UTF-8 string
     *
     * \param [in] buffer null-terminated string to check
     * \return true, buffer points to a valid UTF-8 string
     * \return false, buffer points to an invalid UTF-8 string
     ********************************************************************************/
    [[nodiscard]] bool is_valid_utf8(const code_unit* buffer) noexcept;

    /********************************************************************************
     * \brief Bidirectional iterator for iterating over a UTF-8 string
     *
     ********************************************************************************/
    class iterator final
    {
        const code_unit* ptr; ///< Pointer to a code unit in a UTF-8 string
    public:

        // member types
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = code_point;
        using pointer           = value_type*;
        using reference         = value_type&;

        /********************************************************************************
         * \brief Constructs a null utf8 iterator
         *
         ********************************************************************************/
        iterator() noexcept;

        /********************************************************************************
         * \brief Constructs a utf8 iterator from a pointer
         *
         ********************************************************************************/
        iterator(const code_unit*) noexcept;

        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        /********************************************************************************
         * \brief Increments iterator to refer to next code point in the string
         *
         * \return iterator& this
         ********************************************************************************/
        iterator& operator++() noexcept(!Envy::debug);

        /********************************************************************************
         * \brief Increments iterator to refer to next code point in the string
         *
         * \return iterator copy of this
         ********************************************************************************/
        iterator operator++(int) noexcept(!Envy::debug);

        /********************************************************************************
         * \brief Decrements iterator to point at previous code point
         *
         * \return iterator& this
         ********************************************************************************/
        iterator& operator--() noexcept(!Envy::debug);

        /********************************************************************************
         * \brief Decrements iterator to point at previous code point
         *
         * \return iterator this copy
         ********************************************************************************/
        iterator operator--(int) noexcept(!Envy::debug);

        /********************************************************************************
         * \brief Decodes the current code point
         *
         * \return value_type code point
         ********************************************************************************/
        [[nodiscard]] value_type operator*() const noexcept(!Envy::debug);

        /********************************************************************************
         * \brief Equality comparison
         *
         * \return true The iterators are pointing to the same code point
         * \return false The iterators are pointing to different code points
         ********************************************************************************/
        [[nodiscard]] bool operator==(const iterator&) const noexcept;

        // iterator non-member funcs

        /********************************************************************************
         * \brief Gets the underlying pointer of a utf8 iterator
         *
         * \return const code_unit* pointer
         ********************************************************************************/
        [[nodiscard]] friend const code_unit* iterator_ptr(const iterator&) noexcept;

        /********************************************************************************
         * \brief Returns the distance in bytes between two utf8 iterators
         *
         * \return usize distance
         ********************************************************************************/
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