#include "Render/Textures/TextureFailure.h"

#include <cstdlib>
#include <fstream>
#include <string>
#include <string_view>

namespace
{
std::ofstream trace;
bool chooseYes = true;
void Mark(const char* event)
{
    trace << event << '\n' << std::flush;
}
bool Show(const wchar_t*, bool)
{
    Mark("dialog");
    return chooseYes;
}
const Render::Textures::TextureErrorActions actions{
    &Show,
    []() { Mark("connection"); },
    []() { Mark("sound"); },
    []() { Mark("window"); },
};
} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }
    const std::string_view mode(argv[1]);
    if (mode == "fatal")
    {
        Render::Textures::ExitAfterTextureError();
    }
    if (mode == "continue")
    {
        return Render::Textures::ContinueAfterTextureError(false, true) &&
                       !Render::Textures::ContinueAfterTextureError(false, false) &&
                       !Render::Textures::ContinueAfterTextureError(true, true)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }
    if (mode == "dispatch-continue" || mode == "dispatch-fatal" || mode == "dispatch-decline")
    {
        chooseYes = mode != "dispatch-decline";
        trace.open("texture_failure_" + std::string(mode) + ".txt", std::ios::trunc);
        if (!trace)
        {
            return EXIT_FAILURE;
        }
        Render::Textures::DispatchTextureError(L"bitmap failure", mode == "dispatch-fatal", actions);
        return mode == "dispatch-continue" ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    return EXIT_FAILURE;
}
