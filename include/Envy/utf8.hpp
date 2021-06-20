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

#include "buffers.hpp"

#include <string>
#include <iterator>

namespace Envy::utf8
{

        i32 bytes_encoded(u8 lead_unit);
        i32 bytes_required(code_point cp);
        i32 count_characters();

        bool is_lead_unit(u8 unit) noexcept;
        bool is_continuation_unit(u8 lead_unit) noexcept;
        bool is_valid_char(u8* lead) noexcept;

        void increment_ptr(u8** lead_ptr);
        void decrement_ptr(u8** lead_ptr);

        code_point decode(u8* lead);
        void encode(u8* start, code_point cp);

        code_point next_char(u8** lead_ptr);
        code_point prev_char(u8** lead_ptr);

}