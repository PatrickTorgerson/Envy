#include <macro.hpp>

#include <exception>
#include <utility>
#include <ctype.h>
#include <format>

namespace Envy
{

    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TU Locals ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {
        macro_map global_macros;

        struct macro_tag
        {
            Envy::string name;
            Envy::string param;
            utf8::iterator end;
        };
    }

    Envy::string build_fmt(Envy::string_view fmt)
    {
        Envy::string result { Envy::string::reserve_tag, fmt.size_bytes() + 3u };
        result += "{:";
        result += fmt;
        result += '}';
        return result;
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Envy::macro_map ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void macro_map::add(Envy::string_view name, macro_t m)
    {
        macros.insert({Envy::string(name),m});
    }


    //**********************************************************************
    void macro_map::remove(const Envy::string& name)
    {
        macros.erase(name);
    }


    //**********************************************************************
    void macro_map::clear()
    {
        macros.clear();
    }


    //**********************************************************************
    macro_expantion_result macro_map::expand(const Envy::string& name, Envy::string_view fmt) const
    {
        auto it {macros.find(name)};

        if(it == macros.end())
        { return { "", false }; }

        return { it->second(fmt), true };
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Macro Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

    // -- Helper forwards
    macro_tag read_tag(utf8::iterator start, utf8::iterator end);
    bool is_identifier_string(Envy::string_view s) noexcept;
    char peek(utf8::iterator i, utf8::iterator end);


    //**********************************************************************
    void macro(Envy::string_view name, macro_t m)
    {
        global_macros.add(name, m);
    }


    //**********************************************************************
    macro_expantion_result expand_local_macros(Envy::string_view s, const macro_map& map)
    {
        Envy::string result;
        result.reserve(s.size_bytes() + 10u);

        bool success {true};

        for(auto i {s.begin()}; i != s.end(); ++i)
        {
            if(*i == '{')
            {
                // peek for escapededness
                if( peek(i,s.end()) == '{')
                {
                    result += *i;
                    ++i;
                    continue;
                }

                // read the tag mofo
                auto [identifier,fmt,tag_end] { read_tag(i,s.end()) };

                if(is_identifier_string(identifier))
                {
                    // expand nested macros
                    fmt = expand_local_macros(fmt, map);

                    auto replacement { map.expand(identifier, fmt) };

                    if(replacement.success)
                    {
                        // expand recursive macros , append to result
                        result += expand_local_macros(replacement, map);
                        i = tag_end;
                    }
                    else
                    { result += *i; success = false; }
                }
                else // invalid macro tag, write it to the result as-is
                { result += *i; success = false; }
            }
            // handle end of escape sequence
            // TODO: could break in edge case : "{x:{y}}" where both x and y don't exist
            else if(*i == '}' && peek(i,s.end()) == '}')
            { result += *i; ++i; }
            else // reqular character, add to result
            { result += *i; }
        }

        return { std::move(result), success };
    }


    //**********************************************************************
    macro_expantion_result expand_macros(Envy::string_view s)
    {
        return expand_local_macros(s, global_macros);
    }


    //**********************************************************************
    macro_expantion_result expand_macros(Envy::string_view s, const macro_map& additional)
    {
        return expand_local_macros( expand_local_macros(s, global_macros), additional);
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    macro_tag read_tag(utf8::iterator start, utf8::iterator end)
    {
        macro_tag tag;

        // jump over open curly
        auto i {++start};

        // -- read identifier

        for(; i != end; ++i)
        {
            // beak to read param
            if(*i == ':')
            { break; }

            // no param, return tag
            if(*i == '}')
            {
                tag.param = "";
                tag.end = i;
                return tag;
            }

            tag.name += *i;
        }

        // tag end or param start not found, return invalid tag
        if(i == end)
        { return {"","",end}; }


        // -- read param

        // jump over colon
        ++i;

        // keeps track of nested macro depth
        int nest {};

        for(; i != end; ++i)
        {
            if(*i == '{')
            {
                ++nest;
            }

            if(*i == '}')
            {
                if(nest == 0)
                {
                    tag.end = i;
                    return tag;
                }
                else --nest;
            }

            tag.param += *i;
        }

        // tag end not found, return invalid tag
        return {"","",end};
    }


    //**********************************************************************
    bool is_identifier_string(Envy::string_view s) noexcept
    {
        if(s.empty())
        { return false; }

        if(isdigit((int)s.front().get()))
        { return false; }

        for(auto c : s)
        {
            if(!isalnum((int)c.get()) && c != '_' && c != '-')
            { return false; }
        }

        return true;
    }


    //**********************************************************************
    char peek(utf8::iterator i, utf8::iterator end)
    {
        ++i;
        if(i >= end)
        { return '\0'; }
        else return (char) *utf8::iterator_ptr(i);
    }

}