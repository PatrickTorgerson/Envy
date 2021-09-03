#include <string_view.hpp>
#include <string.hpp>

namespace Envy
{

    //**********************************************************************
    string_view::string_view(const char* data) noexcept :
        size_             { utf8::size_bytes(data) },
        ptr_              { reinterpret_cast<const utf8::code_unit*>(data) },
        code_point_count  { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(const Envy::string& str) noexcept :
        size_{str.size_bytes()},
        ptr_{str.data()},
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(const std::string& str) noexcept :
        size_{str.size()},
        ptr_{ (const utf8::code_unit*) str.data() },
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(const std::string_view& str) noexcept :
        size_{str.size()},
        ptr_{ reinterpret_cast<const utf8::code_unit*>(str.data()) },
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(const utf8::code_unit* data) noexcept :
        size_{ utf8::size_bytes(data) },
        ptr_{data},
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(const char* data, usize size) noexcept :
        size_{size},
        ptr_{ reinterpret_cast<const utf8::code_unit*>(data) },
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(const utf8::code_unit* data, usize size) noexcept :
        size_{size},
        ptr_{data},
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::string_view(utf8::iterator first, utf8::iterator last) noexcept :
        size_ { utf8::iterator_distance_bytes(last, first) },
        ptr_  { utf8::iterator_ptr(first) },
        code_point_count { string::npos }
    {}


    //**********************************************************************
    string_view::operator std::string_view() const
    { return { reinterpret_cast<const char*>(ptr_), size_ }; }


    //**********************************************************************
    const utf8::code_unit* string_view::data() const noexcept
    { return ptr_; }


    //**********************************************************************
    usize string_view::size_bytes() const noexcept
    { return size_; }


    //**********************************************************************
    std::basic_string_view<utf8::code_unit> string_view::code_units() const
    { return std::basic_string_view<utf8::code_unit>(ptr_, size_); }


    //**********************************************************************
    utf8::iterator string_view::begin() const noexcept
    { return cbegin(); }


    //**********************************************************************
    utf8::iterator string_view::end() const noexcept
    { return cend(); }


    //**********************************************************************
    utf8::iterator string_view::cbegin() const noexcept
    { return utf8::iterator(ptr_); }


    //**********************************************************************
    utf8::iterator string_view::cend() const noexcept
    { return utf8::iterator(ptr_ + size_); }


    //**********************************************************************
    utf8::reverse_iterator string_view::rbegin() const noexcept
    { return std::make_reverse_iterator(end()); }


    //**********************************************************************
    utf8::reverse_iterator string_view::rend() const noexcept
    { return std::make_reverse_iterator(begin()); }


    //**********************************************************************
    utf8::reverse_iterator string_view::crbegin() const noexcept
    { return std::make_reverse_iterator(cend()); }


    //**********************************************************************
    utf8::reverse_iterator string_view::crend() const noexcept
    { return std::make_reverse_iterator(cbegin()); }


    //**********************************************************************
    bool string_view::empty() const noexcept
    { return size_ == 0; }


    //**********************************************************************
    usize string_view::size() const noexcept(!Envy::debug)
    {
        if(code_point_count == string::npos)
        {
            code_point_count = utf8::count_code_points(ptr_, size_);
        }

        return code_point_count;
    }


    //**********************************************************************
    utf8::code_point string_view::front() const noexcept(!Envy::debug)
    { return utf8::decode(ptr_); }


    //**********************************************************************
    utf8::code_point string_view::back() const noexcept(!Envy::debug)
    {
        auto p {ptr_ + size_};
        utf8::decrement_ptr(&p);
        return utf8::decode(p);
    }


    //**********************************************************************
    string_view string_view::view(utf8::iterator first, utf8::iterator last) const
    {
        return { first, last };
    }


    //**********************************************************************
    string_view string_view::view_from(utf8::iterator first) const
    {
        return { first, end() };
    }


    //**********************************************************************
    string_view string_view::view_until(utf8::iterator last) const
    {
        return { begin(), last };
    }


    //**********************************************************************
    bool string_view::operator==(const Envy::string_view& other) const noexcept
    {
        return
            size_ == other.size_ &&
            std::strncmp( (const char*) ptr_, (const char*) other.ptr_, size_) == 0;
    }

}