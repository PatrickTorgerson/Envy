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
#include "log.hpp"

#include <typeinfo>
#include <concepts>
#include <functional>

namespace Envy
{
    using event_type  = std::size_t;
    using listener_id = std::size_t;


    template <typename T>
    event_type type()
    { return typeid(T).hash_code(); }


    class event
    {
    public:
        virtual ~event() = default;
    };


    using event_callback = std::function<void(const event*)>;

    // internal
    void register_callback_impl(event_type event_ty, event_callback callback);
    void register_listener(listener_id listenerid, event_type event_ty, event_callback callback);
    void unregister_listener(listener_id listenerid);
    void queue_event(event_type event_ty, const event* e);


    void dispach_events();


    template <std::derived_from<event> E, typename ... Ts>
    void post_event(Ts&& ... args)
    {
        queue_event( type<E>(), new E {std::forward<Ts>(args)...} );
    }

    template <std::derived_from<event> E, typename F>
    requires std::regular_invocable<F,const E&>
    void register_callback(F&& f)
    {
        register_callback_impl( type<E>(), [f](const event* e){ std::invoke(f, *dynamic_cast<const E*>(e)); });
    }


    inline listener_id next_listener_id {1};


    template <typename T>
    class listener
    {
        const listener_id id {0};
    public:

        listener() : id {next_listener_id++}
        {
            register_listener(id, type<T>(), (event_callback) [this](const event* e){ this->recieve( *dynamic_cast<const T*>(e)); });
        }


        virtual ~listener()
        {
            unregister_listener(id);
        }


        virtual void recieve(const T&) = 0;
    };
}