#include <macro.hpp>

#include <exception>
#include <utility>
#include <ctype.h>
#include <format>

namespace Envy
{

    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Macro State ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {
        Envy::macro_map global_macros;

        struct macro_tag
        {
            Envy::string name;
            Envy::string param;
            utf8::iterator end;
        };
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

    // Implements logic ffor expanding macros, 'use_global' determines whether the global macro map is searched for macros as well
    static [[nodiscard]] macro_expantion_result expand_macros_impl(Envy::string_view s, std::initializer_list<std::reference_wrapper<const macro_map>> maps, bool use_global);

    // Searches maps for macro m, returns result of expantion, 'use_global' determines whether the global macro map is searched for macros as well
    static [[nodiscard]] macro_expantion_result expand_macro(macro_tag m, std::initializer_list<std::reference_wrapper<const macro_map>> maps, bool use_global);

    // reads tag a position 'start', returns macro_tag containing parsed information
    static [[nodiscard]] macro_tag read_tag(utf8::iterator start, utf8::iterator end);

    // Returns whether s represents a valid macro identifier
    static [[nodiscard]] bool is_identifier_string(Envy::string_view s) noexcept;

    // returns one code_unit past 'i' or '\0' if i is the last code unit
    static [[nodiscard]] char peek(utf8::iterator i, utf8::iterator end);


    //**********************************************************************
    void macro(Envy::string_view name, macro_t m)
    { global_macros.add(name, m); }


    //**********************************************************************
    macro_expantion_result expand_local_macros(Envy::string_view s, std::initializer_list<std::reference_wrapper<const macro_map>> maps)
    { return expand_macros_impl(s, maps, false); }


    //**********************************************************************
    macro_expantion_result expand_local_macros(Envy::string_view s, const macro_map& map)
    { return expand_macros_impl(s, {std::cref(map)}, false); }


    //**********************************************************************
    macro_expantion_result expand_macros(Envy::string_view s, std::initializer_list<std::reference_wrapper<const macro_map>> maps)
    { return expand_macros_impl(s, maps, true); }


    //**********************************************************************
    macro_expantion_result expand_macros(Envy::string_view s, const macro_map& map)
    { return expand_macros_impl(s, {std::cref(map)}, true); }


    //**********************************************************************
    macro_expantion_result expand_macros(Envy::string_view s)
    { return expand_macros_impl(s, {}, true); }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    macro_expantion_result expand_macros_impl(Envy::string_view s, std::initializer_list<std::reference_wrapper<const macro_map>> maps, bool use_global)
    {
        Envy::string result {Envy::string::reserve_tag, s.size_bytes() + 10u};

        bool success {true};

        for(auto i {s.begin()}; i != s.end(); ++i)
        {
            if(*i == '{')
            {
                // check for open escape sequence '{{'
                if( peek(i,s.end()) == '{' )
                {
                    // we simply replace '{{' with '{'
                    result += '{';
                    ++i;
                    continue;
                }

                // read the tag mofo
                auto tag { read_tag(i,s.end()) };

                if(is_identifier_string(tag.name))
                {
                    // expand nested macros
                    auto fmt = expand_macros_impl(tag.param, maps, use_global);

                    if(fmt.success)
                    {
                        tag.param = std::move(fmt.result);

                        // Search maps for macro, expand
                        auto replacement { expand_macro(tag, maps, use_global) };

                        if(replacement.success)
                        {
                            // expand recursive macros , append to result
                            result += expand_macros_impl(replacement, maps, use_global);
                            i = tag.end;
                            continue;
                        }
                    }
                }

                // -- If we reached this point it means this tag in not valid

                // if(tag.name.empty() || tag.name.contains_only("0123456789"))
                // {
                //     // probably std::format() replacement field
                //     // write to result as-is for std::format to do it's thing
                //     result.append(i,++tag.end);
                // }
                // else
                // {
                //     // probably not std::format() replacement field
                //     // escape so subsequent calls to std::format() don't throw
                //     result += '{';
                //     result.append(i,++tag.end);
                //     result += '}';
                // }

                // write to result as-is
                result.append(i,++tag.end);

                // jump over bad tag, clear success flag
                i = --tag.end;
                success = false;
            }

            // check for close escape sequence '}}'
            else if(*i == '}' && peek(i,s.end()) == '}')
            { result += '}'; ++i; }

            else // reqular character, add to result as-is
            { result += *i; }
        }

        return { std::move(result), success };
    }


    //**********************************************************************
    macro_expantion_result expand_macro(macro_tag m, std::initializer_list<std::reference_wrapper<const macro_map>> maps, bool use_global)
    {
        macro_expantion_result r {};

        if(use_global)
        {
            r = std::move( global_macros.expand(m.name, m.param));

            if(r.success)
            { return r; }
        }

        for(const auto& map : maps)
        {
            r = std::move( map.get().expand(m.name, m.param));

            if(r.success)
            { return r; }
        }

        return {};

    }


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
        { return {"!!","",end}; }


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
        return {"!!","",end};
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


    //**********************************************************************
    Envy::string build_fmt_str(Envy::string_view fmt)
    {
        Envy::string result { Envy::string::reserve_tag, fmt.size_bytes() + 3u };
        result += "{:";
        result += fmt;
        result += '}';
        return result;
    }

}