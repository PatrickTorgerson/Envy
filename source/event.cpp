#include <event.hpp>

#include <map>
#include <memory>
#include <mutex>

namespace Envy
{

    namespace
    {
        std::multimap<event_type,event_callback> func_callbacks;
        std::map<listener_id,event_callback> listener_callbacks;
        std::multimap<event_type, listener_id> listeners;

        using event_queue_entry = std::pair<event_type, std::unique_ptr<const event>>;
        std::vector<event_queue_entry> event_queue;
        std::mutex event_queue_mutex;
    }


    void register_callback_impl(event_type event_ty, event_callback callback)
    {
        func_callbacks.emplace(event_ty, callback);
    }


    void register_listener(listener_id listenerid, event_type event_ty, event_callback callback)
    {
        listener_callbacks.insert(std::make_pair(listenerid,callback));
        listeners.insert(std::make_pair(event_ty,listenerid));
    }


    void unregister_listener(listener_id listenerid)
    {
        listener_callbacks.erase(listenerid);

        for(auto ls_iter {listeners.begin()}; ls_iter != listeners.end();)
        {
            auto& [ty,id] {*ls_iter};

            if(listenerid == id)
            {
                ls_iter = listeners.erase(ls_iter);
            }
            else { ls_iter++; }
        }
    }


    void queue_event(event_type event_ty, const event* e)
    {
        std::scoped_lock<std::mutex> l {event_queue_mutex};
        event_queue.emplace_back(event_ty, std::unique_ptr<const event>(e));
    }


    void dispach_events()
    {
        std::scoped_lock<std::mutex> l {event_queue_mutex};

        for(auto& [ty,event] : event_queue)
        {
            // -- function callbacks

            // range of callbacks interested in this event
            auto [func_begin,func_end] {func_callbacks.equal_range(ty)};

            for(auto func_iter {func_begin}; func_iter != func_end; ++func_iter)
            {
                std::invoke(func_iter->second, event.get());
            }

            // -- listener callbacks

            // range of listeners interested in this event
            auto [listener_begin,listener_end] {listeners.equal_range(ty)};

            for(auto listener_iter {listener_begin}; listener_iter != listener_end; ++listener_iter)
            {
                listener_id listenerid {listener_iter->second};
                std::invoke(listener_callbacks[listenerid], event.get());
            }
        }

        event_queue.clear();
    }
}