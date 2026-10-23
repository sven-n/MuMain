// doctest unit tests for the control socket's `target` field.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Control targets"

#include "stdafx.h"

#include "doctest.h"

#include "App/Control/ControlProtocol.h"
#include "App/Control/ControlTargets.h"

#include <map>
#include <string>

using App::Control::Request;
using App::Control::Targets::ReadMonsterNumber;

namespace
{
// The fields of the request's `target` object, as the commands read them.
std::map<std::string, std::string> TargetFields(const char* line)
{
    const Request request = Request::Parse(line);
    std::map<std::string, std::string> fields;
    REQUIRE(request.GetStringMap("target", fields));
    return fields;
}
} // namespace

TEST_CASE("Control targets read a monster number [network][control-targets]")
{
    int number = -1;

    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":251}})"), number));
    CHECK(number == 251);
    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":0}})"), number));
    CHECK(number == 0);
    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":1023}})"), number));
    CHECK(number == 1023);
}

TEST_CASE("Control targets refuse anything else as a monster number [network][control-targets]")
{
    int number = 7;
    for (const char* line : {
             R"({"cmd":"attack","target":{"monster":1024}})",
             R"({"cmd":"attack","target":{"monster":-1}})",
             R"({"cmd":"attack","target":{"monster":251.5}})",
             R"({"cmd":"attack","target":{"monster":"Hanzo"}})",
             R"({"cmd":"attack","target":{"monster":null}})",
             R"({"cmd":"attack","target":{"monster":251,"name":"Hanzo"}})",
             R"({"cmd":"attack","target":{"number":251}})",
             R"({"cmd":"attack","target":{}})",
         })
    {
        CAPTURE(line);
        CHECK_FALSE(ReadMonsterNumber(TargetFields(line), number));
        CHECK(number == 7);
    }
}

TEST_CASE("Control targets answer a malformed monster target as malformed [network][control-targets]")
{
    const Request request = Request::Parse(R"({"cmd":"attack","target":{"monster":5000}})");
    int key = 0;

    CHECK(App::Control::Targets::ResolveTarget(request, key) == App::Control::Targets::TargetLookup::Malformed);
    CHECK(key == -1);
}
