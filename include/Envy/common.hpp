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

#include <cstdint>
#include <cstddef>
#include <version>


// Ensure build support for required features

#if !defined(__cpp_concepts) || !defined(__cpp_lib_concepts)
    #error "Envy requires C++20 concepts"
#endif
#if !defined(__cpp_lib_three_way_comparison)
    #error "Envy requires three-way-comparison operator (<=>)"
#endif
#if !defined(__cpp_constexpr)
    #error "Envy requires constexpr"
#endif
#if !defined(__cpp_if_constexpr)
    #error "Envy requires constexpr if"
#endif
#if !defined(__cpp_designated_initializers)
    #error "Envy requires designated initializers"
#endif
#if !defined(__cpp_designated_initializers)
    #error "Envy requires designated initializers"
#endif
#if !defined(__cpp_lib_source_location)
    #error "Envy requires source location"
#endif
#if !__has_cpp_attribute(nodiscard)
    #error "Envy requires nodiscard"
#endif
#if !defined(__cpp_lib_format)
    #error "Envy requires format"
#endif
#if !defined(__cpp_lib_string_view)
    #error "Envy requires string_view"
#endif
#if !defined(__cpp_lib_span)
    #error "Envy requires span"
#endif
#if !defined(__cpp_lib_ranges)
    #error "Envy requires ranges"
#endif
#if !defined(__cpp_lib_chrono)
    #error "Envy requires chrono"
#endif
#if !defined(__cpp_lib_math_constants)
    #error "Envy requires math constants"
#endif
#if !defined(__cpp_lib_filesystem)
    #error "Envy requires filesystem"
#endif


#if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
    #define ENVY_PLATFORM_WINDOWS
#else
    #error This operating system is not supported by Envy
#endif


#if defined(N_DEBUG)
    #define ENVY_DEBUG
    #define ENVY_DEBUG_CALL(x) (x)
#else
    #define ENVY_RELEASE
    #define ENVY_DEBUG_CALL(x)
#endif


using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;

using code_point = u32;
using char8 = char8_t;

static_assert(sizeof(f32) == 4u && sizeof(f64) == 8u , "Compiler not supported!");

namespace Envy
{
    constexpr i32 version_major {0};
    constexpr i32 version_minor {0};
    constexpr i32 version_patch {0};

    #if defined(ENVY_DEBUG)
    constexpr bool debug {true};
    #else
    constexpr bool debug {false};
    #endif
}
