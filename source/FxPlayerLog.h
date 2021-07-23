/**********************************************************************

Filename    :   FxPlayerLog.h
Content     :   GFxLog with color fomatting and counts.
Created     :   January 15, 2008
Authors     :   Michael Antonov, Andrew Reisse, Maxim Didenko

Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_FxPlayerLog_H
#define INC_FxPlayerLog_H

#include "GFxLog.h"
#include "GMsgFormat.h"

#ifdef GFC_OS_WIN32
#include <Windows.h>
#endif

// We support color-formatted log only on PC/MAC/Linux platforms,
// on consoles we use regular log through vprintf.
#if defined (GFC_OS_WIN32) || defined(GFC_OS_LINUX) || defined(GFC_OS_UNIX) || defined(GFC_OS_MAC)

#if !defined(GFC_OS_WIN32)
#include <unistd.h> // Because of isatty().
#endif

/*  GFxPlayerLog - Output log with message counter filtering.

This class implements GFxLog by outputting messages to screen or a redirected
file; it receives messages by overriding the LogMessageVarg virtual function.

If repetitive script error messages arrive, they are combined into one and a
counter is displayed next to a message. Such message counting and filtering
is done DisplayCountedMessage. If counting of messages is not necessary,
vprintf(pfmt, argList) could be used instead of DisplayCountedMessage.
*/

class GFxPlayerLog : public GFxLog
{
public:

    // Variables used for repeated message detection.
    enum { Log_MaxBuffer = 2048 };
    char    LastMessageBuffer[Log_MaxBuffer];
    int     LastMessageLength;
    int     LastMessageCounter;
    int     CounterStrLength;
    bool    LastMessageHadNewline;
    bool    IsFileMode;

#ifdef GFC_OS_WIN32
    // Colors used.
    enum ConsoleColor
    {
        ConsoleColor_Gray   = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE,
        ConsoleColor_White  = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
        ConsoleColor_Yellow = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY,
        ConsoleColor_Cyan   = FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY
    };
    // Console output handle and active colors.
    HANDLE          hOutput;
    ConsoleColor    Color, LastColor;

    bool GetFileMode()
    {
        hOutput    = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD Mode;
        return GetConsoleMode(hOutput, &Mode) == 0;
    }

    // Delayed color update - useful to reduce overhead of SetConsoleTextAttribute calls.
    void BeginColor()
    {
        if (!IsFileMode && Color != LastColor)
        {
            ::SetConsoleTextAttribute(hOutput, (WORD)Color);
            LastColor = Color;
        }
    }
    void EndColor()
    {
    }

#else
    // Colors used.
    enum ConsoleColor
    {
        ConsoleColor_Gray   = 0,
        ConsoleColor_White  = 37 | 0x280100,
        ConsoleColor_Yellow = 33 | 0x280100,
        ConsoleColor_Cyan   = 36 | 0x100,
    };
    // Console output handle and active colors.
    ConsoleColor    Color, LastColor;

    bool GetFileMode()
    {
        return 0==isatty(1);
    }

    void BeginColor()
    {
        if (IsFileMode)
            return;

        if (Color & 0xff0000)
            printf("\x1b[%02d;%02d;%02dm", (Color >> 8) & 0xff, (Color >> 16), Color & 0xff);
        else
            printf("\x1b[%02d;%02dm", (Color >> 8) & 0xff, Color & 0xff);
    }
    void EndColor()
    {
        if (IsFileMode)
            return;

        printf("\x1b[00m");
    }

#endif

    void SetTextColor(ConsoleColor attributes)
    {
        Color = attributes;
    }

    GFxPlayerLog()
    {
        LastMessageLength = 0;
        LastMessageHadNewline = 1;

        // We do custom processing if we are in console (screen-output) mode.
        //  - In screen mode we display the message immediately and use '\b'
        //    character to overwrite the message counter as duplicate messages arrive.
        //  - In file mode, '\b' doesn't work, so we wait until the next message
        //    before outputting the counter.
        IsFileMode = GetFileMode();
        // Default colors.
        Color = LastColor = ConsoleColor_Gray;
    }

    ~GFxPlayerLog()
    {
        // Finish last counted log line, if any.
        if (LastMessageLength && LastMessageHadNewline)
        {
            if (IsFileMode && (LastMessageCounter > 1))
                DisplayCounter(LastMessageCounter);
            LogPrint("\n",1);
        }
        SetTextColor(ConsoleColor_Gray);
        BeginColor();
    }

    void LogPrint (const char* buff, UInt length = 0)
    {
#ifdef GFC_OS_WIN32
        if (IsFileMode)
        {
            fputs(buff,stdout);
        }
        else
        {
            if (!length)
                length = (UInt)G_strlen(buff);

            HANDLE hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );

            wchar_t buf[256];
            wchar_t* pbuf;
            if (length < sizeof(buf)/sizeof(buf[0]))
                pbuf = buf;
            else
                pbuf = (wchar_t*)GALLOC(sizeof(buf[0])*(length + 1), GStat_Default_Mem);

            length = (UInt)GUTF8Util::DecodeString(pbuf, buff, length);
            DWORD ret;
            WriteConsoleW(hConsoleOutput, pbuf, length, &ret, NULL);

            if (pbuf != buf)
                GFREE(pbuf);
        }
#else
        GUNUSED(length);
        fputs(buff,stdout);
#endif
    }

    static bool MessageNeedsCounting(LogMessageType messageType)
    {
        return (messageType == Log_ScriptError) || (messageType == Log_ScriptWarning);
    }


    // Displays counter message and returns the number of characters in it.
    static int  DisplayCounter(int counter, int backspaceCount = 0)
    {
        char    backspaceBuff[16], counterBuff[32];
        char*   pbackspace;
        int     i, counterLength;

        // Concatenate backspaces to output string; it's too slow to display them
        // individually through printf.
        for (i = backspaceCount, pbackspace = backspaceBuff; i > 0; pbackspace++,  i--)
            *pbackspace = '\b';
        *pbackspace = 0;

        counterLength = (int)G_Format(
            GStringDataPtr(counterBuff, sizeof(counterBuff)),
            "{0} (x{1})",
            backspaceBuff,
            counter
            );

        printf("%s", counterBuff);
        return counterLength - backspaceCount;
    }

    void    DisplayCountedMessage(LogMessageType messageType, const char* pfmt, va_list argList)
    {
        BeginColor();
        char    messageBuffer[Log_MaxBuffer];
        int     messageLength;

        // If previous message exist and we don't need counting, just flush and output.
        if (!MessageNeedsCounting(messageType))
        {
            if (LastMessageLength && LastMessageHadNewline)
            {
                //printf("\n");
                LogPrint("\n",1);
                LastMessageLength = 0;
            }
            // Output new message.
            messageLength = (int)G_vsprintf(messageBuffer, Log_MaxBuffer-1, pfmt, argList);
            //vprintf(pfmt, argList);
            LogPrint(messageBuffer, messageLength);
            EndColor();
            return;
        }

        // Messages might need counting, so do so.

        messageLength = (int)G_vsprintf(messageBuffer, Log_MaxBuffer-1, pfmt, argList);

        if (messageLength > 0)
        {
            messageBuffer[messageLength] = 0;

            if (!LastMessageHadNewline)
            {
                // If last message didn't have a newline, just concatenate directly.
                // This is a part of the previous message and we don't count these.
                //printf("%s", messageBuffer);
                LogPrint(messageBuffer, messageLength);
                if (messageBuffer[messageLength-1] == '\n')
                {
                    LastMessageHadNewline = 1;
                    LastMessageLength = 0;
                }
            }
            else
            {
                bool haveNewline = 0;
                if (messageBuffer[messageLength-1] == '\n')
                {
                    messageBuffer[messageLength-1] = 0;
                    haveNewline = 1;
                }

                // Consider counting logic.
                if (!LastMessageLength || !haveNewline)
                {
                    // No last message, just output us as-is.
                    //printf("%s", messageBuffer);
                    LogPrint(messageBuffer, messageLength);
                    G_strcpy(LastMessageBuffer, Log_MaxBuffer, messageBuffer);
                    LastMessageLength       = messageLength;
                    LastMessageCounter      = 1;
                    LastMessageHadNewline   = haveNewline;
                    CounterStrLength        = 0;
                }
                else
                {
                    // We had last message: see if they are the same.
                    if (G_strcmp(LastMessageBuffer, messageBuffer) == 0)
                    {
                        // Strings match, so erase old count and update counter.
                        LastMessageCounter++;

                        // Update and display new counter.
                        if (!IsFileMode)
                            CounterStrLength = DisplayCounter(LastMessageCounter, CounterStrLength);
                    }
                    else
                    {
                        // No match. Output newline followed by the new string.
                        if (IsFileMode && (LastMessageCounter > 1))
                            DisplayCounter(LastMessageCounter);

                        //printf("\n%s", messageBuffer);
                        LogPrint("\n",1);
                        LogPrint(messageBuffer, messageLength);

                        G_strcpy(LastMessageBuffer, Log_MaxBuffer, messageBuffer);
                        LastMessageLength       = messageLength;
                        LastMessageCounter      = 1;
                        LastMessageHadNewline   = 1;
                        CounterStrLength        = 0;
                    }
                }
            }
        }

        EndColor();
    }


    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(LogMessageType messageType, const char* pfmt, va_list argList)
    {
        // Output log to console.
        // Highlight script messages for readability.
        if (messageType & Log_Channel_Script)
        {
            if (messageType == Log_ScriptMessage)
                SetTextColor(ConsoleColor_Yellow);
            else // brighter white
                SetTextColor(ConsoleColor_White);
        }
        else if (messageType == Log_Message)
        {   // FsCommand only, player does not generate messages
            // (only errors, warnings, and parse statements).
            SetTextColor(ConsoleColor_Cyan);
        }

        // Non-counted alternative: vprintf(pfmt, argList).
        DisplayCountedMessage(messageType, pfmt, argList);

        // Back to normal.
        if ((messageType & Log_Channel_Script) || (messageType == Log_Message))
            SetTextColor(ConsoleColor_Gray);
    }
};

#else

class GFxPlayerLog : public GFxLog
{
public:
    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(LogMessageType messageType, const char* pfmt, va_list argList)
    {
        // Output log to console
        vprintf(pfmt, argList);
    }
};

#endif

#endif // INC_FxPlayerLog_H
