#include <macro.hpp>

#include <exception>
#include <utility>
#include <ctype.h>
#include <format>

namespace Envy
{

    namespace
    {
        macro_map global_macros;

        struct macro_tag
        {
            std::string name;
            std::string param;
            usize size;
        };
    }


    void macro_map::add(std::string_view name, macro_t m)
    {
        macros[std::string(name)] = m;
    }


    void macro_map::remove(const std::string& name)
    {
        macros.erase(name);
    }


    void macro_map::clear()
    {
        macros.clear();
    }


    macro_expantion_result macro_map::expand(const std::string& name, std::string_view fmt) const
    {
        auto it {macros.find(name)};

        if(it == macros.end())
        { return { "", false }; }

        return { it->second(fmt), true };
    }


    void macro(std::string_view name, macro_t m)
    { global_macros.add(name, m); }


    macro_tag read_tag(std::string_view::const_iterator start, std::string_view::const_iterator end)
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
                tag.size = i - start;
                return tag;
            }

            tag.name.push_back(*i);
        }

        // tag end or param start not found, return invalid tag
        if(i == end)
        { return {}; }

        // jump over colon
        ++i;

        // -- read param

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
                    tag.size = i - start;
                    return tag;
                }
                else --nest;
            }

            tag.param.push_back(*i);
        }

        // tag end not found, return invalid tag
        return {};
    }


    bool is_identifier_string(std::string_view s) noexcept
    {
        if(s.empty())
        { return false; }

        if(isdigit((int)s.front()))
        { return false; }

        for(auto c : s)
        {
            if(!isalnum((int)c) && c != '_' && c != '-')
            { return false; }
        }

        return true;
    }


    char peek(std::string_view::const_iterator i, std::string_view::const_iterator end)
    {
        if(i+1 == end)
        { return '\0'; }
        else return *(i+1);
    }


    macro_expantion_result expand_local_macros(std::string_view s, const macro_map& map)
    {
        std::string result;
        result.reserve(s.size() + 10u);

        bool success {true};

        for(auto i {s.begin()}; i != s.end(); ++i)
        {
            if(*i == '{')
            {
                // peek for escapededness
                if( peek(i,s.end()) == '{')
                {
                    result.push_back(*i);
                    ++i;
                    continue;
                }

                // read the tag mofo
                auto [identifier,fmt,tag_size] { read_tag(i,s.end()) };

                if(is_identifier_string(identifier))
                {
                    // expand nested macros
                    fmt = expand_local_macros(fmt, map);

                    auto replacement { map.expand(identifier, fmt) };

                    if(replacement.success)
                    {
                        // expand recursive macros , append to result
                        result += expand_local_macros(replacement, map);
                        i += tag_size + 1;
                    }
                    else
                    { result.push_back(*i); success = false; }
                }
                else // invalid macro tag, write it to the result as-is
                { result.push_back(*i); success = false; }
            }
            // handle end of escape sequence
            // TODO: could break in edge case : "{x:{y}}" where both x and y don't exist
            else if(*i == '}' && peek(i,s.end()) == '}')
            { result.push_back(*i); ++i; }
            else // reqular character, add to result
            { result.push_back(*i); }
        }

        return { std::move(result), success };
    }


    macro_expantion_result expand_macros(std::string_view s)
    {
        return expand_local_macros(s, global_macros);
    }


    macro_expantion_result expand_macros(std::string_view s, const macro_map& additional)
    {
        return expand_local_macros( expand_local_macros(s, global_macros), additional);
    }

}