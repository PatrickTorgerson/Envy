///////////////////////////////////////////////////////////////////////////////////////
//
//    Envy Game Engine
//    https://github.com/PatrickTorgerson/Envy
//
//    Copyright (c) 2018 Patrick Torgerson
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
#include "math.hpp"

#include <cmath>
#include <concepts>
#include <format>

namespace Envy
{

    template <Envy::numeric T>
    class vector2
    { public: // Access for all!


        // TODO: reflection


        T x,y;


        vector2() : x{} , y{} {};
        vector2(T x, T y) : x{x} , y{y} {}


        template <std::convertible_to<T> U>
        vector2(const vector2<U>& other) :
            x {other.x},
            y {other.y}
        {}


        template <std::convertible_to<T> U>
        vector2<T>& operator = (const vector2<U>& other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }


        [[nodiscard]]
        T length_sqrd() const
        { return x*x + y*y; }


        [[nodiscard]]
        T length() const
        { return (T) std::sqrt( (f64) length_sqrd() ); }


        vector2<T>& normalize()
        {
            auto len = length();
            x /= len; y /= len;
            return *this;
        }


        [[nodiscard]]
        vector2<T> normalized() const
        {
            vector2<T> v(*this);
            return v.normalize();
        }


        [[nodiscard]]
        vector2<T> rotated(f64 angle) const
        {
            return vector2<T>
            {
                x * std::cos(angle) - y * std::sin(angle),
                x * std::sin(angle) + y * std::cos(angle)
            };
        }


        template <std::convertible_to<T> U>
        [[nodiscard]]
        vector2<T> rotated(vector2<U> dir) const
        {
            dir.normalize();
            return vector2<T>
            {
                x * dir.x - y * dir.y,
                x * dir.y + y * dir.x
            };
        }


        vector2<T>& rotate(f64 angle)
        {
            *this = this->rotated(angle);
            return *this;
        }


        template <std::convertible_to<T> U>
        vector2<T>& rotate(vector2<U> dir)
        {
            *this = this->rotated(dir);
            return *this;
        }

        // TODO: Test
        [[nodiscard]]
        std::string to_string() const
        {
            return std::format("({},{})",x,y);
        }


        [[nodiscard]]
        vector2<T> perp_cw() const
        { return rotated({0,-1}); }


        [[nodiscard]]
        vector2<T> perp_ccw() const
        { return rotated({0,1}); }


        template <std::convertible_to<T> U>
        vector2<T>& operator += (const vector2<U>& other)
        { x += (T) other.x;  y += (T) other.y;  return *this; }


        template <std::convertible_to<T> U>
        vector2<T>& operator -= (const vector2<U>& other)
        { x -= (T) other.x;  y -= (T) other.y;  return *this; }


        template <std::convertible_to<T> U>
        vector2<T>& operator *= (U scalar)
        { x *= (T) scalar;  y *= (T) scalar;  return *this; }


        template <std::convertible_to<T> U>
        vector2<T>& operator /= (U scalar)
        { x /= (T) scalar;  y /= (T) scalar;  return *this; }


        // inner / dot product

        template <std::convertible_to<T> U>
        [[nodiscard]]
        auto dot(const vector2<U>& other)
        {  return x*other.x + y*other.y;  }


        template <std::convertible_to<T> U>
        auto operator |= (const vector2<U>& other)
        { return this->dot(other); }


        // outer / wedge product

        template <std::convertible_to<T> U>
        [[nodiscard]]
        auto wedge(const vector2<U>& other)
        {  return x*other.y - y*other.x;  }


        template <std::convertible_to<T> U>
        auto operator ^= (const vector2<U>& other)
        { return this->wedge(other); }


        // element-wise multiplication and division


        template <std::convertible_to<T> U>
        vector2<T>& operator *= (const vector2<U>& other)
        { x *= (T) other.x;  y *= (T) other.y;  return *this; }


        template <std::convertible_to<T> U>
        vector2<T>& operator /= (const vector2<U>& other)
        { x /= (T) other.x;  y /= (T) other.y;  return *this; }

        friend bool operator==(const vector2<T>&, const vector2<T>&) = default;
    };


    template <typename T>
    [[deprecated("Envy::vector3 is not yet implemented")]]
    class vector3
    {
        public: T x,y,z;

        vector3() = default;
        vector3(T x, T y, T z) : x{x} , y{y} , z{z} {}

        vector3(vector2<T> xy, T z) : x{xy.x} , y{xy.y} , z{z} {}
        vector3(T x, vector2<T> yz) : x{x} , y{yz.x} , z{yz.y} {}

        friend bool operator==(const vector3<T>&, const vector3<T>&) = default;
    };


    template <typename T>
    [[deprecated("Envy::vector4 is not yet implemented")]]
    class vector4
    {
        public: T x,y,z,w;

        vector4() = default;
        vector4(T x, T y, T z, T w) : x{x} , y{y} , z{z} , w{w} {}

        vector4(vector2<T> xy, T z, T w) : x{xy.x} , y{xy.y} , z{z} , w{w} {}
        vector4(T x, T y, vector2<T> zw) : x{x} , y{y} , z{zw.x} , w{zw.y} {}

        vector4(vector2<T> xy, vector2<T> zw) : x{xy.x} , y{xy.y} , z{zw.x} , w{zw.y} {}

        friend bool operator==(const vector4<T>&, const vector4<T>&) = default;
    };


    // Aliases


    using vec2 = vector2<f64>;
    using vec3 = vector3<f64>;
    using vec4 = vector4<f64>;


    // operators


    template <Envy::numeric T, std::convertible_to<T> U>
    [[nodiscard]]
    auto operator + (const vector2<T>& left, const vector2<U>& right)
    { return vector2(left.x + right.x , left.y + right.y); }


    template <Envy::numeric T, std::convertible_to<T> U>
    [[nodiscard]]
    auto operator - (const vector2<T>& left, const vector2<U>& right)
    { return vector2(left.x - right.x , left.y - right.y); }


    template <Envy::numeric T, std::convertible_to<T> S>
    [[nodiscard]]
    vector2<T> operator * (vector2<T> vec, S scalar)
    { return vec *= scalar; }


    template <Envy::numeric T, std::convertible_to<T> S>
    [[nodiscard]]
    vector2<T> operator / (vector2<T> vec, S scalar)
    { return vec /= scalar; }


    template <Envy::numeric T, std::convertible_to<T> U>
    [[nodiscard]]
    auto operator | (vector2<T> left, const vector2<U>& right)
    { return left |= right; }


    template <Envy::numeric T, std::convertible_to<T> U>
    [[nodiscard]]
    auto operator ^ (vector2<T> left, const vector2<U>& right)
    { return left ^= right; }
}