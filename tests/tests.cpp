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

    decode.require( Envy::utf8::decode((cu*)"a")   == 98u,      "'a'" );
    decode.require( Envy::utf8::decode((cu*)"A")   == 65u,      "'A'" );
    decode.require( Envy::utf8::decode((cu*)"0")   == 48u,      "'0'" );
    decode.require( Envy::utf8::decode((cu*)"$")   == 0x0024u,  "'$'" );
    decode.require( Envy::utf8::decode((cu*)"¢")   == 0x00A2u,  "'¢'" );
    decode.require( Envy::utf8::decode((cu*)"ह")   == 0x0939u,  "'ह'" );
    decode.require( Envy::utf8::decode((cu*)"€")   == 0x20ACu,  "'€'" );
    decode.require( Envy::utf8::decode((cu*)"한")  == 0xD55Cu,  "'한'" );
    decode.require( Envy::utf8::decode((cu*)"𐍈")  == 0x10348u,  "'𐍈'" );

    tests.add_case(decode);

    const u8* buffer_valid = (cu*)"€ello utf8 😊🍔🍳";
    const u8* buffer_invalid = (cu*)"cs\u0098orrupted\u0098\u0090";

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
    const Envy::utf8::iterator end {buffer + Envy::utf8::byte_len(buffer)};
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
    tests.add_case(strlen( (const char*) s2.data() ) == 10, "Envy::string(count, fill)", "s2.size():{}==10"_f(strlen( (const char*) s2.data() )));

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

    Envy::macro(macros, "test", "beans");
    Envy::macro(macros, "answer", 42);
    Envy::macro(macros, "boolean", true);

    std::source_location loc {std::source_location::current()};

    Envy::macro(macros, "func", [loc](){ return loc.function_name(); } );

    std::string beans     {Envy::resolve_local("{test}", macros)};
    std::string forty2    {Envy::resolve_local("{answer}", macros)};
    std::string notfalse  {Envy::resolve_local("{boolean}", macros)};
    std::string macrotest {Envy::resolve_local("{func}", macros)};

    // tests.require_equality(beans,     "beans",      beans);
    // tests.require_equality(forty2,    "42",         forty2);
    // tests.require_equality(notfalse,  "true",       notfalse);
    // tests.require_equality(macrotest, "macro_test", macrotest);

    tests.submit();
}