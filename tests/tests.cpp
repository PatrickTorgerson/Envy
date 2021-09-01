#include "tests.hpp"

#include <Envy/string.hpp>
#include <Envy/macro.hpp>
#include <Envy/utf8.hpp>
#include <ranges>


void utf8_test(Envy::test_state& tests)
{
    tests.start();

    using cu = Envy::utf8::code_unit;

    tests.add_case( Envy::utf8::is_lead_unit((cu*)"\0") && !Envy::utf8::is_continuation_unit((cu*)"\0") , "Null terminator must be considered a lead unit");

    Envy::test_case decode {"Envy::utf8::decode"};

    decode.require( Envy::utf8::decode((cu*)"a")   == 97u,      "'a'" );
    decode.require( Envy::utf8::decode((cu*)"A")   == 65u,      "'A'" );
    decode.require( Envy::utf8::decode((cu*)"0")   == 48u,      "'0'" );
    decode.require( Envy::utf8::decode((cu*)"$")   == 0x0024u,  "'$'" );
    decode.require( Envy::utf8::decode((cu*)"¢")   == 0x00A2u,  "'¢'" );
    decode.require( Envy::utf8::decode((cu*)"ह")   == 0x0939u,  "'ह'" );
    decode.require( Envy::utf8::decode((cu*)"€")   == 0x20ACu,  "'€'" );
    decode.require( Envy::utf8::decode((cu*)"한")  == 0xD55Cu,  "'한'" );
    decode.require( Envy::utf8::decode((cu*)"𐍈")  == 0x10348u,  "'𐍈'" );

    tests.add_case(decode);

    const u8* buffer_valid { (cu*)"€ello utf8 😊🍔🍳" };

    const u8 buffer_invalid[] { 'a' , 0b11100000u , 0b10000000u , 0b11000010u , 'x' , '\0' };

    tests.add_case(Envy::utf8::is_valid_utf8(buffer_valid) && !Envy::utf8::is_valid_utf8(buffer_invalid), "Envy::utf8::is_valid_utf8");
    tests.add_case(Envy::utf8::count_code_points(buffer_valid) == 14, "Envy::utf8::count_characters(buffer_valid)");

    Envy::utf8::increment_ptr(&buffer_valid);
    Envy::utf8::increment_ptr(&buffer_valid);

    tests.add_case(*buffer_valid == 'l', "increment_ptr(&buffer_valid)");

    Envy::utf8::decrement_ptr(&buffer_valid);

    tests.add_case(*buffer_valid == 'e', "decrement_ptr(&buffer_valid)");

    // -- iterator

    Envy::utf8::code_unit* buffer {(cu*)".𐍈.𐍈.𐍈."};

    const Envy::utf8::iterator begin {buffer};
    const Envy::utf8::iterator end   {buffer + Envy::utf8::size_bytes(buffer)};
    i32 index {};

    Envy::test_case itercase {"Envy::utf8::iterator"};

    for(auto i {begin}; i != end; ++i)
    {
        if(index % 2 == 0) itercase.require(*i == 46u, "'.'");
        if(index % 2 != 0) itercase.require(*i == 0x10348u, "'𐍈'");
        ++index;
    }

    tests.add_case(itercase);

    tests.submit();
}


void string_test(Envy::test_state& tests)
{
    tests.start();

    Envy::string str1 {"hello"};

    Envy::test_case asciichars {"ascii characters"};
    for(auto c : str1)
    {
        asciichars.require( c < 128u, "{} < 128"_f((u32)c));
    }
    tests.add_case(asciichars);

    Envy::string str2 {"¢😊한🍔🍳𐍈"};

    Envy::test_case utf8char {"unicode characters"};
    for(auto c : str2)
    {
        utf8char.require(c > 128u, "{} > 128"_f((u32)c));
    }
    tests.add_case(utf8char);

    tests.add_case(str1.size() == 5 && str2.size() == 6, "Envy::string::size()", "str1:{}==5 , str2:{}==6"_f(str1.size(), str2.size()));

    Envy::string s2 {10, 'a'};
    tests.add_case(Envy::utf8::size_bytes(s2.data()) == 10, "Envy::string(count, fill)", "s2.size():{}==10"_f( Envy::utf8::size_bytes(s2.data())));

    Envy::string s3 {};

    s3.append("1");
    s3.append(std::string("2"));

    tests.add_case(s3 == "12", "Envy::string::append()");

    Envy::string s4 {u8"look for the needle in the haystack"};
    Envy::string s5 {u8"needle"};

    auto subrange = std::ranges::search(s4, s5);

    using namespace std::string_literals;

    Envy::test_case search {"std::ranges::search()"};

    search.require(!subrange.empty(),          "empty");
    search.require(*subrange.begin()   == 'n', "begin");
    search.require(*(--subrange.end()) == 'e', "end");
    search.require(subrange.front()    == 'n', "front");
    search.require(subrange.back()     == 'e', "back");

    tests.add_case(search);

    tests.submit();
}


void string_view_test(Envy::test_state& tests)
{
    tests.start();

    Envy::string string {"A big boi utf8 string Ω π 😁"};
    Envy::string_view view1 {string};

    auto end { std::ranges::find(string, 'u') };
    --end;

    Envy::string_view view2 { string.begin(), end };
    Envy::string_view view3 { string.view_until(end) };
    Envy::string_view view4 { string.view(std::ranges::find(string, 'u') , -- std::ranges::find(string, "Ω")) };

    i32 ascii {};
    i32 utf8  {};

    for(auto cp : view1)
    {
        if(cp < 128u)
        { ++ascii; }
        else
        { ++utf8; }
    }

    Envy::string s;
    for(auto cp : view4 | std::views::reverse)
    {
        s.append(cp);
    }
    tests.add_case( s == "gnirts 8ftu" , "reverse iterators", s.c_str());

    s.append('g');

    tests.add_case( ascii == 24 && utf8 == 3 , "iteration", "ascii:{}==24 , utf8:{}==3"_f(ascii,utf8));
    tests.add_case( view2 == view3, "equality", "view2 != view3");
    tests.add_case( view2.size() == view2.size_bytes(), "size" , "chars:{} , bytes:{}"_f(view2.size(), view2.size_bytes()) );

    tests.submit();
}


void vector_test(Envy::test_state& tests)
{
    tests.start();
    tests.submit();
}


void unicode_test(Envy::test_state& tests)
{
    tests.start();

    tests.submit();
}


void buffers_test(Envy::test_state& tests)
{
    tests.start();
    tests.submit();
}


void macro_test(Envy::test_state& tests)
{
    tests.start();

    Envy::macro_map macros;

    macros.add("test", "beans");
    macros.add("answer", 42);
    macros.add("boolean", true);
    macros.add("pi", 3.1415926535);

    macros.add("first-name", "Patrick");
    macros.add("last-name", "Torgerson");
    macros.add("full-name", "{first-name} {last-name}");

    std::source_location loc {std::source_location::current()};

    macros.add("func", [loc](Envy::string_view){ return loc.function_name(); } );

    macros.add("len", [](Envy::string_view p){ return Envy::to_string(p.size()); });

    std::string beans     {Envy::expand_local_macros("{test}", macros)};
    std::string forty2    {Envy::expand_local_macros("{answer}", macros)};
    std::string notfalse  {Envy::expand_local_macros("{boolean}", macros)};
    std::string macrotest {Envy::expand_local_macros("{func}", macros)};
    std::string pi        {Envy::expand_local_macros("{pi:.2f}", macros)};
    std::string torgerson {Envy::expand_local_macros("{len:{last-name}}", macros)};
    std::string name      {Envy::expand_local_macros("{full-name}", macros)};
    std::string beanlen   {Envy::expand_local_macros("{len:{test}}", macros)};

    tests.add_case(beans == "beans",              "test -> beans = {}"_f(beans));
    tests.add_case(forty2 == "42",                "answer -> 42 = {}"_f(forty2));
    tests.add_case(notfalse == "true",            "boolean -> true = {}"_f(notfalse));
    tests.add_case(macrotest == "macro_test",     "func -> macro_test = {}"_f(macrotest));
    tests.add_case(pi == "3.14",                  "pi -> pi = {}"_f(pi));
    tests.add_case(torgerson == "9",              "len:last-name -> 9 = {}"_f(torgerson));
    tests.add_case(name == "Patrick Torgerson",   "full-name -> Patrick Torgerson = {}"_f(name));
    tests.add_case(beanlen == "5",                "len:test -> 5 = {}"_f(beanlen));

    // TODO: test expanding with multiple macro_maps

    tests.submit();
}