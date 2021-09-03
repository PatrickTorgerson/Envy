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

/********************************************************************************
 * \file macro.hpp
 * \brief Utilities for expanding macros in strings
 ********************************************************************************/

#pragma once

#include "common.hpp"
#include "string.hpp"
#include "log.hpp"

#include <functional>
#include <unordered_map>
#include <optional>

namespace Envy
{
    // TODO: Use Envy::string

    /********************************************************************************
     * \brief The type for a macro function
     ********************************************************************************/
    using macro_t = std::function<Envy::string(Envy::string_view)>;

    /********************************************************************************
     * \brief Function pointer type for macro function, used to aid overload resolution
     ********************************************************************************/
    using raw_macro_t = Envy::string (*) (Envy::string_view);


    Envy::string build_fmt_str(Envy::string_view fmt);


    /********************************************************************************
     * \brief Return type for macro expantion functions
     *
     * Contains members for the result string and a flag indicating whether all macros
     * were successfully expanded. can be implicitly cast to
     * a string to get the result string.
     *
     * \see Envy::expand_macros()
     * \see Envy::expand_local_macros()
     ********************************************************************************/
    class macro_expantion_result final
    {
    public:

        Envy::string result {""};  ///< The result of the macro expantion, unexpanded macros will be left as is.
        bool success  {false};     ///< Success flag, true if all macros were expanded, false otherwise


        macro_expantion_result() = default;

        /********************************************************************************
         * \brief Constructs a macro expantion result
         *
         * \param [in] str rvalue ref, result string
         * \param [in] s success flag
         ********************************************************************************/
        macro_expantion_result(Envy::string&& str, bool s) : result{std::move(str)} , success{s} {}

        macro_expantion_result(macro_expantion_result&&) = default;
        macro_expantion_result& operator=(macro_expantion_result&&) = default;


        /********************************************************************************
         * \brief Implicit cast to Envy::string with macros expanded
         *
         * This overload is called on rvalue macro_expantion_result 's and will move
         * the result string.
         *
         * \return Envy::string&& The result of the macro expantion, unexpanded macros will be left as is.
         ********************************************************************************/
        operator Envy::string&& () && { return std::move(result); }


        /********************************************************************************
         * \brief Implicit cast to Envy::string with macros expanded
         *
         * This overload is called on lvalue macro_expantion_result 's and will copy
         * the result string.
         *
         * \return Envy::string&& The result of the macro expantion, unexpanded macros will be left as is.
         ********************************************************************************/
        operator Envy::string   () const &  { return result; }


        /********************************************************************************
         * \brief Implicit cast to std::string with macros expanded
         * \return std::string, The result of the macro expantion, unexpanded macros will be left as is.
         ********************************************************************************/
        operator std::string() const { return static_cast<std::string>(result); }


        /********************************************************************************
         * \brief Implicit cast to std::string with macros expanded
         * \return std::string, The result of the macro expantion, unexpanded macros will be left as is.
         ********************************************************************************/
        operator std::string_view() const { return static_cast<std::string_view>(result); }


        /********************************************************************************
         * \brief Implicit cast to Envy::string_view with macros expanded
         *
         * It is the programmer's responsibility to ensure that the resulting string view does not outlive the string.
         *
         * ```cpp
         * // Bad: sv holds a dangling pointer
         * std::string_view sv = Envy::expand_macros("Hello {player}, want to {activity}?");
         * ```
         *
         * \return Envy::string_view A view into the result string
         ********************************************************************************/
        operator Envy::string_view() const& { return static_cast<Envy::string_view>(result); }


        /********************************************************************************
         * \brief Grants access to the result string without having to cast to string
         *
         * \return Envy::string* pointer to result string
         ********************************************************************************/
        Envy::string* operator->() { return &result; }


        /********************************************************************************
         * \brief Grants access to the result string without having to cast to string
         *
         * \return const Envy::string* pointer to const result string
         ********************************************************************************/
        const Envy::string* operator->() const { return &result; }

    };


    /********************************************************************************
     * \brief Container of macros
     * \see Envy::expand_macros()
     * \see Envy::expand_local_macros()
     ********************************************************************************/
    class macro_map final
    {

        std::unordered_map<Envy::string,macro_t> macros; ///<< underlying map of macros

    public:

        /********************************************************************************
         * \brief Adds a function macro to the map
         *
         * Adds a macro that expands to a the returned string of a macro function.
         * A macro function has the signiture "Envy::string(Envy::string_view param)"
         * where param is the text after the colon in a macro tag, "{name:param}"
         *
         * ```cpp
         * std::string len_macro(sttd::string param)
         * {
         *     return Envy::to_string(param.size());
         * }
         *
         * macros.add("len", len_macro);
         *
         * Envy::expand_local_macros("Length: {len:Torgerson}");
         * // returns "Length: 9"
         * ```
         *
         * \param [in] name name of the new macro
         * \param [in] m macro function
         ********************************************************************************/
        void add(Envy::string_view name, macro_t m);
        void add(Envy::string_view name, raw_macro_t m)
        { add(name, macro_t {m}); }


        /********************************************************************************
         * \brief Adds a value macro to the map
         *
         * Adds a macro that expands to a value converted to a string with std::format()
         * where param is the format specifier.
         *
         * ```cpp
         * Envy::macro_map macros;
         * macros.add("pi", 3.1415926535);
         *
         * Envy::info( Envy::expand_local_macros("Pi, nearest hundredth : {pi:.2f}") );
         * ```
         *
         * \tparam T Type of the value, must be convertible to string
         * \param [in] name name of the new macro
         * \param [in] v value
         ********************************************************************************/
        template <convertable_to_string T>
        void add(Envy::string_view name, T&& v)
        {
            add( name,
                [ t = std::forward<T>(v) ] (Envy::string_view fmt)
                { return std::format( build_fmt_str(fmt) , t); }
            );
        }


        /********************************************************************************
         * \brief Removes a macro from the map
         *
         * If macro does not exist, does nothing
         *
         * \param [in] name macro to remove
         ********************************************************************************/
        void remove(const Envy::string& name);


        /********************************************************************************
         * \brief Removes all macros from the map
         ********************************************************************************/
        void clear();


        /********************************************************************************
         * \brief Returns the value of a macro
         *
         * If macro does not exist, the result string will be empty and the success flag
         * will be false.
         *
         * ```cpp
         * auto result = macros.expand("pi");
         *
         * if(result.success)
         * {
         *     str += result;
         * }
         * ```
         *
         * \param [in] name macro to expand
         * \param [in] fmt fmt to expand the macro with as if "{name:fmt}"
         * \return Envy::macro_expantion_result, the result of the expantion
         *
         * \see Envy::macro_expantion_result
         ********************************************************************************/
        macro_expantion_result expand(const Envy::string& name, Envy::string_view fmt = "") const;

    };


    /********************************************************************************
     * \brief Adds a function macro to the global macro map
     *
     * Adds a macro that expands to a the returned string of a macro function.
     * A macro function has the signiture "std::string(std::string param)"
     * where param is the text after the colon in a macro tag, "{name:param}"
     *
     * ```cpp
     * std::string len_macro(sttd::string param)
     * {
     *     return Envy::to_string(param.size());
     * }
     *
     * macros.add("len", len_macro);
     *
     * Envy::expand_local_macros("Length: {len:Torgerson}");
     * // returns "Length: 9"
     * ```
     *
     * \param [in] name name of the new macro
     * \param [in] m
     ********************************************************************************/
    void macro(Envy::string_view name, macro_t m);
    inline void macro(Envy::string_view name, raw_macro_t m)
    { macro(name, macro_t {m}); }


    /********************************************************************************
     * \brief Adds a value macro to the global macro map
     *
     * Adds a macro that expands to a value converted to a string with std::format()
     * where param is the format specifier.
     *
     * ```cpp
     * Envy::macro_map macros;
     * macros.add("pi", 3.1415926535);
     *
     * Envy::info( Envy::expand_local_macros("Pi, nearest hundredth : {pi:.2f}") );
     * ```
     *
     * \tparam T Type of the value, must be convertible to string
     * \param [in] name name of the new macro
     * \param [in] v value
     ********************************************************************************/
    template <convertable_to_string T>
    void macro(Envy::string_view name, T&& v)
    {
        macro( name,
            [ t = std::forward<T>(v) ] (Envy::string_view fmt)
            { return std::format( build_fmt_str(fmt) , t); }
        );
    }


    /********************************************************************************
     * \brief Expand macros in string, excluding global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \param [in] s String to expand
     * \param [in] maps List of \ref Envy::macro_map 's to search when expanding macros
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    [[nodiscard]] macro_expantion_result expand_local_macros(Envy::string_view s, std::initializer_list<std::reference_wrapper<const macro_map>> maps);


    /********************************************************************************
     * \brief Expand macros in string, excluding global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \param [in] s String to expand
     * \param [in] map \ref Envy::macro_map to search when expanding macros
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    [[nodiscard]] macro_expantion_result expand_local_macros(Envy::string_view s, const macro_map& map);


    /********************************************************************************
     * \brief Expand macros in string, excluding global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \tparam Maps template parameter pack of Envy::macro_map 's
     * \param [in] s
     * \param [in] maps List of \ref Envy::macro_map 's to search when expanding macros
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    template < std::same_as<macro_map> ... Maps >
    [[nodiscard]] macro_expantion_result expand_local_macros(Envy::string_view s, const Maps& ... maps)
    { return expand_local_macros(s, {std::cref(maps)...}); }


    /********************************************************************************
     * \brief Expand macros in string, including global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \param [in] s String to expand
     * \param [in] maps List of additional \ref Envy::macro_map 's to search when expanding macros
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    [[nodiscard]] macro_expantion_result expand_macros(Envy::string_view s, std::initializer_list<std::reference_wrapper<const macro_map>> maps);


    /********************************************************************************
     * \brief Expand macros in string, including global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \param [in] s String to expand
     * \param [in] map Additional \ref Envy::macro_map to search when expanding macros
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    [[nodiscard]] macro_expantion_result expand_macros(Envy::string_view s, const macro_map& map);


    /********************************************************************************
     * \brief Expand macros in string, including global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \param [in] s String to expand
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    [[nodiscard]] macro_expantion_result expand_macros(Envy::string_view s);


    /********************************************************************************
     * \brief Expand macros in string, including global macros
     *
     * *s* may contain zero of more **Macro Tags**
     * which if found in any of the provided Envy::macro_map 's will be replaced
     * with the expanded result of that macro.
     *
     * \tparam Maps template parameter pack of Envy::macro_map 's
     * \param [in] s String to expand
     * \param [in] maps List of additional Envy::macro_map 's to search when expanding macros
     * \return Envy::macro_expantion_result
     ********************************************************************************/
    template < std::same_as<macro_map> ... Maps >
    [[nodiscard]] macro_expantion_result expand_macros(Envy::string_view s, const Maps& ... maps)
    { return expand_macros(s, {std::cref(maps)...}); }

}