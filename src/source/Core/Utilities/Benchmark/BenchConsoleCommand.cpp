#include "stdafx.h"

#include "BenchConsoleCommand.h"

#include "BenchRecorder.h"
#include "BenchSegment.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/NewUI/NewUISystem.h"

#include <sstream>
#include <vector>

namespace Core::Benchmark::Console
{
namespace
{
    constexpr const wchar_t* kCommandPrefix = L"$bench";

    // A shorter, noisier run for iterating on one effect: enough frames to see a large change,
    // not enough to trust a small one. The full defaults are what a reported number comes from.
    constexpr int kQuickRepeats = 1;
    constexpr int kQuickWarmupFrames = 30;
    constexpr int kQuickMeasureFrames = 120;

    void Say(const std::wstring& text)
    {
        g_pChatListBox->AddText(L"", text.c_str(), SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    void Say(const std::string& text)
    {
        Say(std::wstring(text.begin(), text.end()));
    }

    std::vector<std::wstring> SplitWords(const std::wstring& text)
    {
        std::wistringstream stream(text);
        std::vector<std::wstring> words;
        std::wstring word;
        while (stream >> word) words.push_back(word);
        return words;
    }

    std::string Narrow(const std::wstring& text)
    {
        std::string out;
        for (wchar_t c : text) out += (c > 0 && c < 128) ? (char)c : '?';
        return out;
    }

    int ParsePositive(const std::wstring& text, int fallback)
    {
        try
        {
            const int value = std::stoi(text);
            return (value > 0) ? value : fallback;
        }
        catch (const std::exception&)
        {
            return fallback;
        }
    }

    void PrintUsage()
    {
        Say(L"$bench list                     - the segment catalog");
        Say(L"$bench run [pattern] [n] [f]    - run segments, n repeats of f frames");
        Say(L"$bench quick [pattern]          - one short repeat, for iterating");
        Say(L"$bench label <text>             - name the next run");
        Say(L"$bench stop                     - end the current run early");
    }

    void PrintCatalog()
    {
        for (const Segment& segment : Segments::All())
            Say(std::string(segment.name) + "  [" + Segments::TagsToString(segment.tags) + "]");
        Say(L"patterns: 'fx.*', '*.off', '#particles', '*'");
    }

    void StartRun(const std::vector<std::wstring>& words, int repeats, int warmupFrames, int measureFrames)
    {
        const std::string pattern = (words.size() > 2) ? Narrow(words[2]) : "*";

        std::string message;
        if (!Recorder::Instance().Start(pattern, repeats, warmupFrames, measureFrames, message))
        {
            Say("benchmark: " + message);
            return;
        }
        Say(message);

        // Said at the start, not only in the finished report: under vsync every segment's frame
        // time is pinned to the display refresh, so the segments come out looking alike whatever
        // they switch off. Learning that from the report costs a whole run.
        if (IsVSyncEnabled())
            Say(L"benchmark: vsync is ON -- frame times are pinned to the refresh rate and the "
                L"segments will look alike. Run '$vsync off' and measure again.");
    }

    void RunWithArguments(const std::vector<std::wstring>& words)
    {
        const int repeats = (words.size() > 3)
            ? ParsePositive(words[3], Recorder::kDefaultRepeats) : Recorder::kDefaultRepeats;
        const int measureFrames = (words.size() > 4)
            ? ParsePositive(words[4], Recorder::kDefaultMeasureFrames) : Recorder::kDefaultMeasureFrames;

        StartRun(words, repeats, Recorder::kDefaultWarmupFrames, measureFrames);
    }

    void StopRun()
    {
        Recorder& recorder = Recorder::Instance();
        if (!recorder.IsRunning())
        {
            Say(L"benchmark: nothing is running");
            return;
        }

        recorder.Abort();
        Say("benchmark aborted, partial results in " + recorder.LastOutputPath());
    }

    void SetLabel(const std::wstring& command)
    {
        // Everything after "$bench label ", so a label may contain spaces.
        const size_t labelAt = command.find(L"label ");
        const std::wstring label = (labelAt == std::wstring::npos) ? L"" : command.substr(labelAt + 6);

        Recorder::Instance().SetLabel(Narrow(label));
        Say("benchmark label: " + Narrow(label));
    }
}

bool HandleCommand(const std::wstring& command)
{
    if (command.compare(0, wcslen(kCommandPrefix), kCommandPrefix) != 0) return false;

    const std::vector<std::wstring> words = SplitWords(command);
    const std::wstring verb = (words.size() > 1) ? words[1] : L"";

    if (verb.empty() || verb == L"help") PrintUsage();
    else if (verb == L"list") PrintCatalog();
    else if (verb == L"run") RunWithArguments(words);
    else if (verb == L"quick") StartRun(words, kQuickRepeats, kQuickWarmupFrames, kQuickMeasureFrames);
    else if (verb == L"label") SetLabel(command);
    else if (verb == L"stop") StopRun();
    else Say(L"benchmark: unknown command, try '$bench help'");

    return true;
}
}
