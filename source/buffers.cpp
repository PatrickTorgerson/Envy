#include <buffers.hpp>

namespace Envy
{

    ///////////////////////////////////////////////////////////////////////////////////////
    // -- Envy::const_buffer
    ///////////////////////////////////////////////////////////////////////////////////////

    const u8* const_buffer::data() const noexcept
    { return d; }


    usize const_buffer::size() const noexcept
    { return s; }


    bool const_buffer::empty() const noexcept
    { return s == 0u; }


    const_buffer::operator bool() const noexcept
    { return empty(); }


    u8 const_buffer::operator[](usize i) const noexcept
    { return d[i]; }


    u8 const_buffer::operator*() const noexcept
    { return *d; }


    const_buffer& const_buffer::operator+=(usize offset) noexcept
    {
        d += offset;
        s -= offset;
        return *this;
    }


    void const_buffer::remove_prefix(usize count)
    {
        // TODO: assert(count <= s)
        *this += count;
    }


    void const_buffer::remove_suffix(usize count)
    {
        // TODO: assert(count <= s)
        s -= count;
    }


    u8 const_buffer::first() const noexcept
    { return *d; }


    u8 const_buffer::last() const noexcept
    { return d[s-1u]; }


    const_buffer const_buffer::first(usize count) const
    {
        // TODO: assert(count <= s)
        return const_buffer(d,count);
    }


    const_buffer const_buffer::last(usize count) const
    {
        // TODO: assert(count <= s)
        return const_buffer(d + (s-count), count);
    }


    const_buffer const_buffer::sub(usize pos, usize size) const
    {
        // TODO: assert(pos + size <= s)
        return const_buffer(d + pos, size);
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    // -- Envy::mutable_buffer
    ///////////////////////////////////////////////////////////////////////////////////////


    u8* mutable_buffer::data() noexcept
    { return d; }


    const u8* mutable_buffer::data() const noexcept
    { return d;}


    usize mutable_buffer::size() const noexcept
    { return s; }


    bool mutable_buffer::empty() const noexcept
    { return s == 0u; }


    mutable_buffer::operator bool() const noexcept
    { return empty(); }


    mutable_buffer::operator const_buffer() const noexcept
    { return const_buffer(d,s); }


    u8 mutable_buffer::operator[](usize i) const noexcept
    { return d[i]; }


    u8 mutable_buffer::operator*() const noexcept
    { return *d; }


    u8& mutable_buffer::operator[](usize i) noexcept
    { return d[i]; }


    u8& mutable_buffer::operator*() noexcept
    { return *d; }


    mutable_buffer& mutable_buffer::operator+=(usize offset) noexcept
    {
        d += offset;
        s -= offset;
        return *this;
    }


    void mutable_buffer::remove_prefix(usize count)
    {
        // TODO: assert(count <= s)
        *this += count;
    }


    void mutable_buffer::remove_suffix(usize count)
    {
        // TODO: assert(count <= s)
        s -= count;
    }


    u8 mutable_buffer::first() const noexcept
    { return *d; }


    u8 mutable_buffer::last() const noexcept
    { return d[s-1u]; }


    mutable_buffer mutable_buffer::first(usize count) const
    {
        // TODO: assert(count <= s)
        return mutable_buffer(d,count);
    }


    mutable_buffer mutable_buffer::last(usize count) const
    {
        // TODO: assert(count <= s)
        return mutable_buffer(d+(s-count), count);
    }


    mutable_buffer mutable_buffer::sub(usize pos, usize size) const
    {
        // TODO: assert(pos + count <= s)
        return mutable_buffer(d+pos,size);
    }



}