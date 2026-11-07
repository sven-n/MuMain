#include "stdafx.h"

#include "ReadingLanguageLog.h"

#include "Core/Utilities/Log/MuLogger.h"

namespace Core::Log::ReadingLanguage
{
namespace
{
const char* DescribeItemNames(bool translated)
{
    return translated ? "translated" : "in English";
}

// What MuError.log says the player reads; empty before the start-up line.
std::optional<State>& Logged()
{
    static std::optional<State> logged;
    return logged;
}
} // namespace

std::string Describe(const std::optional<State>& logged, const State& now)
{
    if (!logged)
        return "> UI language = " + now.uiLocale + ", item names " + DescribeItemNames(now.itemNamesTranslated) + ".";

    std::string changes;
    if (logged->uiLocale != now.uiLocale)
        changes = "UI language changed to " + now.uiLocale;
    if (logged->itemNamesTranslated != now.itemNamesTranslated)
        changes += (changes.empty() ? "Item names now " : ", item names now ") +
                   std::string(DescribeItemNames(now.itemNamesTranslated));
    return changes.empty() ? changes : "> " + changes + ".";
}

void Write(const State& now)
{
    std::optional<State>& logged = Logged();
    const std::string line = Describe(logged, now);
    logged = now;
    if (line.empty())
        return;

    // MuError.log's logger, as CErrorReport writes it: flushed per line so a crash keeps the line.
    const auto logger = mu::log::Get("core");
    MU_LOG_INFO(logger, "{}", line);
    logger->flush();
}
} // namespace Core::Log::ReadingLanguage
