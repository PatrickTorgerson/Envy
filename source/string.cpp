#include <string.hpp>

#include <exception>
#include <utility>
#include <cctype>
#include <format>
#include <bit>
#include <cstring>
#include <algorithm>
#include <ranges>

namespace Envy
{

    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Construction ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    string::string(reserve_tag_t, usize bytes) :
        buffer_size      { 0 },
        buffer_capacity  { new_capacity(bytes) },
        buffer           { new utf8::code_unit[buffer_capacity] }
    {
        buffer[0] = '\0';
    }


    //**********************************************************************
    string::string(size_tag_t, usize bytes) :
        buffer_size      { bytes },
        buffer_capacity  { new_capacity(bytes+5) },
        buffer           { new utf8::code_unit[buffer_capacity] }
    {
        buffer[0] = '\0';
        buffer[buffer_size] = '\0';
    }


    //**********************************************************************
    string::string() :
        string(reserve_tag, 30u)
    { }


    //**********************************************************************
    string::string(const char* cstr) :
        string(size_tag, utf8::size_bytes(cstr))
    {
        // +1 to copy null-terminator
        std::memcpy(buffer, cstr, buffer_size + 1u);
    }


    //**********************************************************************
    string::string(const char8_t* cstr) :
        string( reinterpret_cast<const char*>(cstr) )
    { }


    //**********************************************************************
    string::string(usize count, char fill) :
        string(size_tag, count)
    {
        std::fill(buffer, buffer + buffer_size, (u8) fill);
    }


    //**********************************************************************
    string::string(const string& from) :
        buffer_size       { from.buffer_size },
        buffer_capacity   { from.buffer_capacity },
        buffer            { new utf8::code_unit[buffer_capacity] },
        code_point_count  { from.code_point_count }
    {
        std::copy(from.buffer, from.buffer + buffer_size, this->buffer);
        buffer[buffer_size] = '\0';
    }


    //**********************************************************************
    string::string(string&& from) noexcept :
        buffer_size       { from.buffer_size },
        buffer_capacity   { from.buffer_capacity },
        buffer            { from.buffer },
        code_point_count  { from.code_point_count }
    {
        from.buffer_size = 0u;
        from.buffer_capacity = 0u;
        from.buffer = nullptr;
        from.code_point_count = npos;
    }


    //**********************************************************************
    string::string(const std::string& from) :
        buffer_size       { from.size() },
        buffer_capacity   { from.capacity() + 1 },
        buffer            { new utf8::code_unit[buffer_capacity] },
        code_point_count  { npos }
    {
        std::copy(from.data(), from.data() + buffer_size, this->buffer);

        buffer[buffer_size] = '\0';
    }


    //**********************************************************************
    string::string(string_view from) :
        string(size_tag, from.size_bytes())
    {
        std::copy(from.data(), from.data() + buffer_size, this->buffer);
    }


    //**********************************************************************
    string::~string()
    {
        if(buffer)
        { delete[] buffer; }
    }


    //**********************************************************************
    string& string::operator=(const string& from)
    {
        if(&from != this)
        {
            if(buffer)
            { delete[] buffer; }

            buffer_size       = from.buffer_size;
            buffer_capacity   = from.buffer_capacity;
            buffer            = new utf8::code_unit[buffer_capacity];
            code_point_count  = from.code_point_count;

            std::copy(from.buffer, from.buffer + buffer_size, this->buffer);

            buffer[buffer_size] = '\0';
        }
        return *this;
    }


    //**********************************************************************
    string& string::operator=(string&& from) noexcept
    {
        if(&from != this)
        {
            if(buffer)
            { delete[] buffer; }

            buffer_size       = from.buffer_size;
            buffer_capacity   = from.buffer_capacity;
            buffer            = from.buffer;
            code_point_count  = from.code_point_count;

            from.buffer_size       = 0u;
            from.buffer_capacity   = 0u;
            from.buffer            = nullptr;
            from.code_point_count   = npos;
        }
        return *this;
    }


    //**********************************************************************
    string& string::operator=(const std::string& from)
    {
        buffer_size       = from.size();
        buffer_capacity   = from.capacity();
        buffer            = new utf8::code_unit[buffer_capacity];
        code_point_count  = npos;

        std::copy(from.data(), from.data() + buffer_size, this->buffer);
        buffer[buffer_size] = '\0';

        return *this;
    }


    //**********************************************************************
    string::operator std::string_view () const
    {
        return { reinterpret_cast<const char*>(buffer) , buffer_size };
    }


    //**********************************************************************
    string::operator std::string() const
    {
        return { reinterpret_cast<const char*>(buffer) , buffer_size };
    }


    //**********************************************************************
    const utf8::code_unit* string::data() const noexcept
    { return buffer; }


    //**********************************************************************
    usize string::size_bytes() const noexcept
    { return buffer_size; }


    //**********************************************************************
    utf8::iterator string::begin() const noexcept
    { return utf8::iterator(buffer); }


    //**********************************************************************
    utf8::iterator string::end() const noexcept
    { return utf8::iterator(buffer + buffer_size); }


    //**********************************************************************
    utf8::iterator string::cbegin() const noexcept
    { return begin(); }


    //**********************************************************************
    utf8::iterator string::cend() const noexcept
    { return end(); }


    //**********************************************************************
    utf8::reverse_iterator string::rbegin() const noexcept
    { return std::make_reverse_iterator(end()); }


    //**********************************************************************
    utf8::reverse_iterator string::rend() const noexcept
    { return std::make_reverse_iterator(begin()); }


    //**********************************************************************
    utf8::reverse_iterator string::crbegin() const noexcept
    { return std::make_reverse_iterator(cend()); }


    //**********************************************************************
    utf8::reverse_iterator string::crend() const noexcept
    { return std::make_reverse_iterator(cbegin()); }


    //**********************************************************************
    std::basic_string_view<utf8::code_unit> string::code_units() const
    { return std::basic_string_view<utf8::code_unit>(buffer, buffer_size); }


    //**********************************************************************
    const char* string::c_str() const
    { return (const char*) buffer; }


    //**********************************************************************
    string_view string::view(utf8::iterator first, utf8::iterator last) const noexcept
    {
        return { first, last };
    }


    //**********************************************************************
    string_view string::view_from(utf8::iterator first) const noexcept
    {
        return { first, end() };
    }


    //**********************************************************************
    string_view string::view_until(utf8::iterator last) const noexcept
    {
        return { begin(), last };
    }


    //**********************************************************************
    bool string::empty() const noexcept
    { return buffer_size == 0; }


    //**********************************************************************
    usize string::size() const noexcept(!Envy::debug)
    {
        if(code_point_count == npos)
        {
            code_point_count = utf8::count_code_points(buffer);
        }

        return code_point_count;
    }


    //**********************************************************************
    usize string::capacity() const noexcept
    { return buffer_capacity; }


    void string::clear() noexcept
    { *buffer = '\0'; buffer_size = 0; }


    //**********************************************************************
    void string::reserve(usize bytes)
    {
        adjust_buffer(bytes);
    }


    //**********************************************************************
    string& string::append(const char* cstr)
    {
        return append(std::string_view(cstr));
    }


    //**********************************************************************
    string& string::append(Envy::string_view str)
    {
        adjust_buffer( buffer_size + str.size_bytes() );

        for(auto cu : str.code_units())
        {
            buffer[buffer_size] = cu;
            ++buffer_size;
        }

        buffer[buffer_size] = '\0';

        return *this;
    }


    //**********************************************************************
    string& string::append(utf8::code_point cp)
    {
        i32 units {utf8::code_units_required(cp)};
        adjust_buffer( buffer_size + units );

        utf8::encode(cp, buffer + buffer_size);

        buffer_size += units;

        buffer[buffer_size] = '\0';

        return *this;
    }


    //**********************************************************************
    string& string::append(char c)
    {
        adjust_buffer( buffer_size + 1 );
        buffer[buffer_size] = (utf8::code_unit) c;
        ++buffer_size;
        buffer[buffer_size] = '\0';

        return *this;
    }


    //**********************************************************************
    string& string::operator+=(const char* cstr)
    {
        return this->append(cstr);
    }


    //**********************************************************************
    string& string::operator+=(Envy::string_view str)
    {
        return this->append(str);
    }


    //**********************************************************************
    string& string::operator+=(utf8::code_point cp)
    {
        return this->append(cp);
    }


    //**********************************************************************
    string& string::operator+=(char c)
    {
        return this->append(c);
    }


    //**********************************************************************
    bool string::contains(string_view sv) const noexcept
    {
        return ! std::ranges::search(*this, sv).empty();
    }


    //**********************************************************************
    bool string::contains(utf8::code_point cp) const noexcept
    {
        // for(const auto& this_cp : *this)
        // {
        //     if(this_cp == cp)
        //     { return true; }
        // }

        return std::ranges::any_of(*this, [&cp](const utf8::code_point& this_cp){ return this_cp == cp; });
    }


    //**********************************************************************
    bool string::contains_any(string_view sv) const noexcept
    {
        // if(sv.empty())
        // { return true; }

        // for(const auto& cp : *this)
        // {
        //     for(const auto& sv_cp : sv)
        //     {
        //         if(cp == sv_cp)
        //         { return true; }
        //     }
        // }

        return std::ranges::any_of(*this, [&sv](const utf8::code_point& cp){ return sv.contains(cp); });
    }


    //**********************************************************************
    bool string::contains_all(string_view sv) const noexcept
    {
        // if(sv.empty())
        // { return true; }

        // for(const auto& sv_cp : sv)
        // {
        //     bool in {false};

        //     for(const auto& cp : *this)
        //     {
        //         if(cp == sv_cp)
        //         { in = true; }
        //     }

        //     if(!in)
        //     { return false; }
        // }

        return std::ranges::all_of(sv, [this](const utf8::code_point& cp){ return this->contains(cp); });
    }


    //**********************************************************************
    bool string::contains_only(string_view sv) const noexcept
    {
        // if(sv.empty())
        // { return this->empty(); }

        // for(const auto& cp : *this)
        // {
        //     bool in {false};

        //     for(const auto& sv_cp : sv)
        //     {
        //         if(cp == sv_cp)
        //         { in = true; }
        //     }

        //     if(!in)
        //     { return false; }
        // }

        return std::ranges::all_of(*this, [&sv](const utf8::code_point& cp){ return sv.contains(cp); });
    }


    //**********************************************************************
    bool string::operator==(const Envy::string& other) const noexcept
    {
        // TODO: Maybe use std::memcmp()??
        return
            buffer_size == other.buffer_size &&
            std::strcmp( (const char*) buffer, (const char*) other.buffer ) == 0;
    }


    //**********************************************************************
    Envy::string operator+(Envy::string left, Envy::string right)
    {
        Envy::string s { Envy::String::reserve_tag , left.size_bytes() + right.size_bytes() };
        s += left;
        s += right;
        return s;
    }


    //**********************************************************************
    usize string::new_capacity(usize required_size) noexcept
    {
        // plus one to accomodate for null-terminator
        return std::bit_ceil(required_size+1);
    }


    //**********************************************************************
    void string::adjust_buffer(usize required_size)
    {
        if(required_size > buffer_capacity)
        {
            buffer_capacity = new_capacity(required_size);

            utf8::code_unit* new_buffer { new utf8::code_unit[buffer_capacity] };

            if(buffer)
            {
                std::memcpy(new_buffer, buffer, buffer_size+1);
                delete[] buffer;
            }

            buffer = new_buffer;
        }

    }


    //**********************************************************************
    std::string replace(std::string_view str, std::string_view target, std::string_view replacement)
    {
        std::size_t prev {0u};
        std::size_t pos  {str.find(target)};
        std::string result;

        while(pos != std::string::npos)
        {
            result += str.substr(prev, pos - prev);
            result += replacement;

            prev = pos + target.size();
            pos = str.find(target, pos+1);
        }

        result += str.substr(prev);
        return result;
    }

}