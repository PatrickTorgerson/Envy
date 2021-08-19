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
    }

    void macro(macro_map& map, std::string_view name, macro_t m)
    { map[std::string(name)] = m; }


    void macro(std::string_view name, macro_t m)
    { macro(global_macros, name, m); }


    std::tuple<std::string,std::string,usize> read_tag(std::string_view::const_iterator start, std::string_view::const_iterator end)
    {
        std::tuple<std::string,std::string,usize> tag;

        auto i {start};

        // tag identifier
        for(; i != end; ++i)
        {
            if(*i == ':')
            { break; }

            if(*i == '}')
            {
                std::get<1>(tag) = "{}";
                std::get<2>(tag) = i - start;
                return tag;
            }

            std::get<0>(tag).push_back(*i);
        }

        if(i == end)
        { return std::tuple<std::string,std::string,usize> {}; }

        // fmt specifier
        std::get<1>(tag).push_back('{');
        for(; i != end; ++i)
        {
            std::get<1>(tag).push_back(*i);

            if(*i == '}')
            {
                std::get<2>(tag) = i - start;
                return tag;
            }
        }

        return std::tuple<std::string,std::string,usize> {};
    }


    bool is_identifier_string(std::string_view s) noexcept
    {
        if(s.empty())
        { return false; }

        if(isdigit((int)s.front()))
        { return false; }

        for(auto c : s)
        {
            if(!isalnum((int)c) && c != '_')
            { return false; }
        }

        return true;
    }


    std::string resolve_local(std::string_view s, const macro_map& map)
    {
        // TODO: escaped macro tags?

        std::string result;
        result.reserve(s.size());

        for(auto i {s.begin()}; i != s.end(); ++i)
        {
            if(*i == '{')
            {
                ++i;

                if(*i == '}')
                {
                    // empty tag, probably a placeholder for std::format() so we ignore
                    result += "{}";
                }
                else
                {
                    // possible macro tag
                    auto [identifier,fmt,tag_size] {read_tag(i,s.end())};

                    if(is_identifier_string(identifier))
                    {
                        // yes, a valid macro tag (we just assume fmt is valid, let std::format() deal with it)

                        auto macro_it {map.find(identifier)};

                        if(macro_it == map.end())
                        {
                            // macro does not exist, let's just write it to the result as-is
                            result.push_back('{');
                            --i;
                        }
                        else
                        {
                            // let the replacement occur
                            std::string replacement;

                            replacement = resolve_local(macro_it->second(), map);
                            replacement = std::format(fmt, replacement);

                            result += replacement;
                            i += tag_size;
                        }
                    }
                    else
                    {
                        // invalid tag, let's just write it to the result as-is
                        result.push_back('{');
                        --i;
                    }
                }
            }
            else
            { result.push_back(*i); }
        }

        return result;
    }


    std::string resolve(std::string_view s)
    {
        return resolve_local(s, global_macros);
    }


    std::string resolve(std::string_view s, const macro_map& additional)
    {
        return resolve_local( resolve_local(s, additional), global_macros);
    }

}