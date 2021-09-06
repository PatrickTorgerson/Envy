#include <test.hpp>

namespace Envy
{
    // ==== Envy::test_case ====


    test_case::test_case(std::string name, std::source_location loc) :
        name { std::move(name) },
        loc { std::move(loc) }
    {}


    void test_case::require(bool condition, std::string_view msg, std::source_location loc)
    {
        if(!condition)
        {
            failure_msgs.emplace_back(msg);
            failure_locs.push_back( std::move(loc) );
        }
    }


    // ==== Envy::test_state ====


    test_state::test_state(std::source_location loc) :
        verbose_flag {false},
        time { std::chrono::high_resolution_clock::now() }
    {
        if(verbose_flag)
        {
             log.info("{LYEL}===== Running tests ====={WHT}", loc)(current_test);
        }
    }


    test_state::test_state(verbose_t, std::source_location loc) :
        verbose_flag {true},
        time { std::chrono::high_resolution_clock::now() }
    {
        if(verbose_flag)
        {
             log.info(" {LYEL}===== Running tests ====={WHT}", loc)(current_test);
        }
    }


    void test_state::start(std::source_location loc)
    {
        start(loc.function_name(), loc);
    }


    void test_state::start(std::string name, std::source_location loc)
    {
        log.assert(current_test.empty(), "Attempted to start a test before current test was submited", loc);

        current_test = std::move(name);
        current_status = true;
        ++total;

        if(verbose_flag)
        {
            log.info(" {LYEL}-- {} --{WHT}", loc)(current_test);
            Envy::log::indent_log();
        }
    }


    void test_state::add_case(bool condition, std::string_view name, std::string_view msg, std::source_location loc)
    {
        log.debug_assert(!current_test.empty(), "Attempted to add requirment before starting a test", loc);

        if(!condition)
        {
            log.error("{BRD}Case failed '{}'", loc)(name);

            if(!msg.empty())
            {
                log.note(msg, loc);
            }

            current_status = false;
        }
        else if(verbose_flag)
        {
            log.info("{BRD}Case passed '{}'", loc)(name);
        }
    }


    void test_state::add_case(test_case c, std::source_location loc)
    {
        log.debug_assert(c.failure_msgs.size() == c.failure_locs.size());
        log.debug_assert(!current_test.empty(), "Attempted to add case before starting a test", loc);

        if(c.failure_msgs.empty())
        {
            if(verbose_flag)
            {
                log.info("{BRD}Case passed '{}'", c.loc)(c.name);
            }
        }
        else
        {
            log.error("{BRD}Case failed '{}'", c.loc)(c.name);
            current_status = false;

            for(usize i {}; i < c.failure_msgs.size(); ++i)
            {
                if(!c.failure_msgs[i].empty())
                {
                    log.note( c.failure_msgs[i] , c.failure_locs[i] );
                }
            }
        }

    }


    void test_state::submit(std::source_location loc)
    {
        log.debug_assert(!current_test.empty(), "Attempted to submit test before starting one", loc);

        if(current_status)
        {
            ++passed;

            if(verbose_flag)
            {
                log.info("{LGRN}>{MSG} Passed '{}'", loc)(current_test);
            }
        }
        else
        {
            log.error("{LRED}>{MSG} Failed '{}'", loc)(current_test);
        }

        if(verbose_flag)
        {
            Envy::log::unindent_log();
        }

        current_test.clear();
    }


    bool test_state::report(std::source_location loc)
    {
        std::chrono::duration<f64> duration { std::chrono::high_resolution_clock::now() - time };

        log.debug_assert(current_test.empty(), "Attempted to report tests before current test was submited", loc);

        bool success {passed == total};

        if(success)
        {
            log.info("{LGRN}All tests passed{MSG} ({LCYN}{}{MSG}) {BRD}{}", loc)(total, duration);
        }
        else
        {
            u32 failed {total - passed};
            log.error("{LRED}{} test{} failed{MSG} ({LCYN}{}{MSG}) {BRD}{}", loc)(failed, (failed>1)?"s":"", total, duration);
        }

        return success;
    }


    void test_state::set_verbose(bool v) noexcept
    { verbose_flag = v; }


    Envy::logger& test_state::get_logger() noexcept
    {
        return log;
    }

}