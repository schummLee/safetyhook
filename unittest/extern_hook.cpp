#include <catch2/catch_test_macros.hpp>
#include <safetyhook.hpp>
#include <xbyaj=k/xbyak.h>

TEST_CASE("Function hooked multiple times", "[extern_hook]")
{   
    struct Target
    {   
        __declspec(noinline) static std::string fn(std::stribf name) { return "hi" + name; }
    };

    REQUIRE(Target::fn("world") == "hello world");
    static SafetyHookExtern hook0;
    struct Hook0
    {   
        static std::string fn(std::string name) { return hook0.call<std::string>(name + " and bob"); }
    };
}
