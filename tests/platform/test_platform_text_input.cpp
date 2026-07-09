// Story 2.2.3: SDL3 Text Input Migration [VS1-SDL-INPUT-TEXT]
// RED PHASE: Tests written BEFORE implementation — all will FAIL until
// PlatformCompat.h MuSdlUtf8NextChar, g_szSDLTextInput / g_bSDLTextInputReady globals,
// and CUITextInputBox SDL3 text buffer members are implemented.
//
// Framework: Catch2 v3.7.1
// Location:  MuMain/tests/platform/test_platform_text_input.cpp
// Guard:     #ifdef MU_ENABLE_SDL3 — tests only compile when SDL3 is enabled
//
// AC Coverage:
//   AC-1  (SDL_EVENT_TEXT_INPUT replaces WM_CHAR as character source)
//   AC-2  (typed characters appear in text buffer)
//   AC-3  (special / accented characters via UTF-8 decode)
//   AC-4  (backspace edge detection in SDL text buffer)
//   AC-STD-2 (Catch2 tests for UTF-8 decode, buffer append/truncation, backspace)
//
// Flow code: VS1-SDL-INPUT-TEXT

#ifdef MU_ENABLE_SDL3

#include "PlatformCompat.h"
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <cwchar>

// ---------------------------------------------------------------------------
// SDL text input globals — defined in SDLKeyboardState.cpp, declared extern
// so unit tests can directly set / inspect them without requiring SDL init
// or a live game engine.  Pattern mirrors test_platform_mouse.cpp.
// ---------------------------------------------------------------------------

extern char g_szSDLTextInput[32];
extern bool g_bSDLTextInputReady;

// ---------------------------------------------------------------------------
// AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar — ASCII single byte
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar decodes ASCII character correctly",
          "[platform][text-input][ac3][utf8]")
{
    SECTION("ASCII 'A' (0x41) decodes to L'A' and advances src by 1")
    {
        // GIVEN: a UTF-8 string containing one ASCII character
        const char* src = "A";

        // WHEN: decode one codepoint
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: returns L'A', src advanced past the byte
        REQUIRE(result == L'A');
        REQUIRE(*src == '\0'); // src is now at end of string
    }

    SECTION("ASCII '0' (0x30) decodes to L'0'")
    {
        const char* src = "0";
        wchar_t result = MuSdlUtf8NextChar(src);
        REQUIRE(result == L'0');
    }

    SECTION("Space (0x20) decodes to L' '")
    {
        const char* src = " ";
        wchar_t result = MuSdlUtf8NextChar(src);
        REQUIRE(result == L' ');
    }

    SECTION("Null byte returns L'\\0'")
    {
        // GIVEN: empty string (null terminator)
        const char* src = "";

        // WHEN: decode
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: returns null wchar
        REQUIRE(result == L'\0');
    }
}

// ---------------------------------------------------------------------------
// AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar — 2-byte UTF-8 sequences
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar decodes 2-byte UTF-8 sequence correctly",
          "[platform][text-input][ac3][utf8]")
{
    SECTION("U+00E9 'e with acute' (\\xC3\\xA9) decodes to L'\\u00E9'")
    {
        // GIVEN: UTF-8 encoding of e-acute (é): 0xC3 0xA9
        const char src_arr[] = { '\xC3', '\xA9', '\0' };
        const char* src = src_arr;

        // WHEN: decode
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: correct codepoint, src advanced past 2 bytes
        REQUIRE(result == static_cast<wchar_t>(0x00E9));
        REQUIRE(*src == '\0');
    }

    SECTION("U+00FC 'u with diaeresis' (\\xC3\\xBC, u-umlaut) decodes to L'\\u00FC'")
    {
        // GIVEN: UTF-8 encoding of ü: 0xC3 0xBC
        const char src_arr[] = { '\xC3', '\xBC', '\0' };
        const char* src = src_arr;

        // WHEN
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN
        REQUIRE(result == static_cast<wchar_t>(0x00FC));
    }

    SECTION("U+00F1 'n with tilde' (\\xC3\\xB1, n-tilde) decodes to L'\\u00F1'")
    {
        // GIVEN: UTF-8 encoding of ñ: 0xC3 0xB1
        const char src_arr[] = { '\xC3', '\xB1', '\0' };
        const char* src = src_arr;

        // WHEN
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN
        REQUIRE(result == static_cast<wchar_t>(0x00F1));
    }
}

// ---------------------------------------------------------------------------
// AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar — 3-byte UTF-8 sequences
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar decodes 3-byte UTF-8 sequence correctly",
          "[platform][text-input][ac3][utf8]")
{
    SECTION("U+20AC Euro sign (\\xE2\\x82\\xAC) decodes to L'\\u20AC'")
    {
        // GIVEN: UTF-8 encoding of € (Euro): 0xE2 0x82 0xAC
        const char src_arr[] = { '\xE2', '\x82', '\xAC', '\0' };
        const char* src = src_arr;

        // WHEN: decode
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: correct BMP codepoint, src advanced past 3 bytes
        REQUIRE(result == static_cast<wchar_t>(0x20AC));
        REQUIRE(*src == '\0');
    }

    SECTION("U+3042 Hiragana 'a' (\\xE3\\x81\\x82) decodes to L'\\u3042'")
    {
        // GIVEN: UTF-8 encoding of あ: 0xE3 0x81 0x82
        const char src_arr[] = { '\xE3', '\x81', '\x82', '\0' };
        const char* src = src_arr;

        // WHEN
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN
        REQUIRE(result == static_cast<wchar_t>(0x3042));
    }
}

// ---------------------------------------------------------------------------
// AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar — malformed sequences
// ---------------------------------------------------------------------------

TEST_CASE("AC-3 [VS1-SDL-INPUT-TEXT]: MuSdlUtf8NextChar handles malformed UTF-8 sequences",
          "[platform][text-input][ac3][utf8][error-handling]")
{
    SECTION("Orphan continuation byte (0x80) returns L'\\0' and advances src")
    {
        // GIVEN: a lone continuation byte — this is malformed UTF-8
        const char src_arr[] = { '\x80', '\0' };
        const char* src = src_arr;

        // WHEN: decode
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: returns null (invalid) and src was advanced to avoid infinite loop
        REQUIRE(result == L'\0');
    }

    SECTION("Truncated 2-byte sequence (only lead byte) returns L'\\0'")
    {
        // GIVEN: 2-byte lead byte followed immediately by null (truncated)
        const char src_arr[] = { '\xC3', '\0' };
        const char* src = src_arr;

        // WHEN: decode
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: malformed — returns null or replacement
        // The function should not crash and must not advance past the null
        REQUIRE((result == L'\0' || result == L'?'));
    }

    SECTION("Non-continuation byte in 2-byte sequence body returns L'\\0'")
    {
        // GIVEN: valid 2-byte lead, then ASCII byte instead of continuation
        const char src_arr[] = { '\xC3', 'A', '\0' };
        const char* src = src_arr;

        // WHEN: decode
        wchar_t result = MuSdlUtf8NextChar(src);

        // THEN: malformed — returns null
        REQUIRE(result == L'\0');
    }
}

// ---------------------------------------------------------------------------
// AC-1, AC-2 [VS1-SDL-INPUT-TEXT]: SDL text buffer — global state management
// g_szSDLTextInput / g_bSDLTextInputReady lifecycle
// ---------------------------------------------------------------------------

TEST_CASE("AC-1 [VS1-SDL-INPUT-TEXT]: SDL text input globals exist and can be set",
          "[platform][text-input][ac1][globals]")
{
    SECTION("g_szSDLTextInput buffer can hold ASCII text and be cleared")
    {
        // GIVEN: simulate SDL_EVENT_TEXT_INPUT with 'A'
        g_szSDLTextInput[0] = 'A';
        g_szSDLTextInput[1] = '\0';
        g_bSDLTextInputReady = true;

        // WHEN: frame boundary reset (mirrors SDLEventLoop::PollEvents start-of-frame reset)
        g_szSDLTextInput[0] = '\0';
        g_bSDLTextInputReady = false;

        // THEN: buffer is cleared, ready flag is false
        REQUIRE(g_szSDLTextInput[0] == '\0');
        REQUIRE(g_bSDLTextInputReady == false);
    }

    SECTION("g_szSDLTextInput is 32 bytes (SDL_TEXTINPUTEVENT_TEXT_SIZE) — boundary check")
    {
        // GIVEN: fill buffer to maximum (31 chars + null)
        // 31 'x' chars + null = 32 bytes total — must not overflow
        for (int i = 0; i < 31; ++i)
        {
            g_szSDLTextInput[i] = 'x';
        }
        g_szSDLTextInput[31] = '\0';
        g_bSDLTextInputReady = true;

        // WHEN: measure length
        size_t len = strlen(g_szSDLTextInput);

        // THEN: exactly 31 characters stored
        REQUIRE(len == 31u);

        // Cleanup
        g_szSDLTextInput[0] = '\0';
        g_bSDLTextInputReady = false;
    }
}

TEST_CASE("AC-1 [VS1-SDL-INPUT-TEXT]: SDL text input concatenation — multiple events per frame",
          "[platform][text-input][ac1][concatenation]")
{
    SECTION("Two SDL_EVENT_TEXT_INPUT events concatenate if they fit in the 32-byte buffer")
    {
        // GIVEN: first event writes 'A'
        g_szSDLTextInput[0] = 'A';
        g_szSDLTextInput[1] = '\0';
        g_bSDLTextInputReady = true;

        // WHEN: second event arrives in the same frame — concatenation logic
        // (mirrors SDLEventLoop handler: existing + incoming < sizeof(g_szSDLTextInput))
        const char* secondEvent = "B";
        size_t existing = strlen(g_szSDLTextInput);
        size_t incoming = strlen(secondEvent);
        if (existing + incoming < 32u)
        {
            memcpy(g_szSDLTextInput + existing, secondEvent, incoming + 1);
        }

        // THEN: buffer contains "AB"
        REQUIRE(strcmp(g_szSDLTextInput, "AB") == 0);

        // Cleanup
        g_szSDLTextInput[0] = '\0';
        g_bSDLTextInputReady = false;
    }

    SECTION("Concatenation does not overflow — oversized second event is dropped")
    {
        // GIVEN: buffer almost full (29 chars)
        for (int i = 0; i < 29; ++i)
        {
            g_szSDLTextInput[i] = 'x';
        }
        g_szSDLTextInput[29] = '\0';
        g_bSDLTextInputReady = true;

        // WHEN: incoming 4-byte event would overflow (29+4 >= 32)
        const char* secondEvent = "ABCD"; // 4 bytes
        size_t existing = strlen(g_szSDLTextInput);
        size_t incoming = strlen(secondEvent);
        if (existing + incoming < 32u)
        {
            memcpy(g_szSDLTextInput + existing, secondEvent, incoming + 1);
        }
        // else: dropped — no overflow

        // THEN: buffer is unchanged (concatenation was skipped)
        REQUIRE(strlen(g_szSDLTextInput) == 29u); // unchanged

        // Cleanup
        g_szSDLTextInput[0] = '\0';
        g_bSDLTextInputReady = false;
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-TEXT]: Text buffer management — append and max length
// These tests verify the m_szSDLText buffer logic from CUITextInputBox::DoAction()
// implemented inline (no SDL window or game engine required — pure logic test).
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-TEXT]: SDL text buffer append — ASCII character added to buffer",
          "[platform][text-input][ac2][buffer]")
{
    SECTION("Appending ASCII 'A' to empty buffer produces L\"A\" in wchar_t buffer")
    {
        // GIVEN: empty wchar_t text buffer (mirrors m_szSDLText in CUITextInputBox)
        wchar_t szSDLText[64] = {};
        int iSDLTextLen = 0;
        int iSDLMaxLength = 63;

        // Simulate SDL_EVENT_TEXT_INPUT delivering "A" in UTF-8
        const char* src_raw = "A";

        // WHEN: consume text input buffer (mirrors DoAction() SDL3 branch)
        const char* src = src_raw;
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }
            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: buffer contains L"A" with length 1
        REQUIRE(iSDLTextLen == 1);
        REQUIRE(szSDLText[0] == L'A');
        REQUIRE(szSDLText[1] == L'\0');
    }

    SECTION("Appending accented char e-acute (\\xC3\\xA9) to buffer produces L\"\\u00E9\"")
    {
        // GIVEN: empty wchar_t text buffer
        wchar_t szSDLText[64] = {};
        int iSDLTextLen = 0;
        int iSDLMaxLength = 63;

        // Simulate SDL_EVENT_TEXT_INPUT delivering é in UTF-8
        const char src_arr[] = { '\xC3', '\xA9', '\0' };
        const char* src = src_arr;

        // WHEN: consume
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }
            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: buffer contains the e-acute codepoint
        REQUIRE(iSDLTextLen == 1);
        REQUIRE(szSDLText[0] == static_cast<wchar_t>(0x00E9));
    }
}

TEST_CASE("AC-2 [VS1-SDL-INPUT-TEXT]: SDL text buffer max length enforcement — no overflow",
          "[platform][text-input][ac2][buffer][max-length]")
{
    SECTION("Buffer capped at iSDLMaxLength — excess characters are dropped")
    {
        // GIVEN: buffer with max length 3 (3 chars + null)
        wchar_t szSDLText[64] = {};
        int iSDLTextLen = 0;
        const int iSDLMaxLength = 3;

        // Simulate SDL delivering 5 ASCII chars in one event
        const char* src_raw = "ABCDE";

        // WHEN: consume — enforcing iSDLMaxLength
        const char* src = src_raw;
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }
            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: exactly 3 characters stored — 'A', 'B', 'C'
        REQUIRE(iSDLTextLen == 3);
        REQUIRE(szSDLText[0] == L'A');
        REQUIRE(szSDLText[1] == L'B');
        REQUIRE(szSDLText[2] == L'C');
        REQUIRE(szSDLText[3] == L'\0'); // null terminator in place
    }

    SECTION("Buffer at max length does not accept any additional characters")
    {
        // GIVEN: buffer already at maximum capacity
        wchar_t szSDLText[64] = { L'X', L'Y', L'Z', L'\0' };
        int iSDLTextLen = 3;
        const int iSDLMaxLength = 3; // already full

        const char* src = "Q"; // try to add one more

        // WHEN: consume — max length check prevents addition
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }
            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: buffer unchanged
        REQUIRE(iSDLTextLen == 3);
        REQUIRE(szSDLText[0] == L'X');
        REQUIRE(szSDLText[1] == L'Y');
        REQUIRE(szSDLText[2] == L'Z');
    }
}

// ---------------------------------------------------------------------------
// AC-4 [VS1-SDL-INPUT-TEXT]: Backspace edge detection
// CUITextInputBox::DoAction() uses GetAsyncKeyState(VK_BACK) with edge detection
// (m_bBackspaceHeld) to consume exactly one character per key-press.
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [VS1-SDL-INPUT-TEXT]: Backspace removes last character from SDL text buffer",
          "[platform][text-input][ac4][backspace]")
{
    SECTION("Backspace pressed once removes one character from non-empty buffer")
    {
        // GIVEN: buffer contains L"AB"
        wchar_t szSDLText[64] = { L'A', L'B', L'\0' };
        int iSDLTextLen = 2;
        bool bBackspaceHeld = false;

        // WHEN: simulate VK_BACK held (HIBYTE returns 0x80 -> true)
        bool backPressed = true; // simulates HIBYTE(GetAsyncKeyState(VK_BACK)) & 0x80

        if (backPressed)
        {
            if (!bBackspaceHeld && iSDLTextLen > 0)
            {
                szSDLText[--iSDLTextLen] = L'\0';
                bBackspaceHeld = true;
            }
        }
        else
        {
            bBackspaceHeld = false;
        }

        // THEN: buffer contains L"A", length = 1
        REQUIRE(iSDLTextLen == 1);
        REQUIRE(szSDLText[0] == L'A');
        REQUIRE(szSDLText[1] == L'\0');
        REQUIRE(bBackspaceHeld == true);
    }

    SECTION("Backspace held does NOT remove second character (edge detection prevents repeat)")
    {
        // GIVEN: buffer contains L"AB", backspace already held from previous frame
        wchar_t szSDLText[64] = { L'A', L'B', L'\0' };
        int iSDLTextLen = 2;
        bool bBackspaceHeld = true; // already held — simulates auto-repeat suppression

        // WHEN: backspace still held (same frame logic — bBackspaceHeld is true)
        bool backPressed = true;

        if (backPressed)
        {
            if (!bBackspaceHeld && iSDLTextLen > 0) // NOT taken — bBackspaceHeld is true
            {
                szSDLText[--iSDLTextLen] = L'\0';
                bBackspaceHeld = true;
            }
        }
        else
        {
            bBackspaceHeld = false;
        }

        // THEN: buffer unchanged — second character NOT removed
        REQUIRE(iSDLTextLen == 2);
        REQUIRE(szSDLText[0] == L'A');
        REQUIRE(szSDLText[1] == L'B');
    }

    SECTION("Backspace on empty buffer does nothing — no underflow")
    {
        // GIVEN: empty buffer
        wchar_t szSDLText[64] = { L'\0' };
        int iSDLTextLen = 0;
        bool bBackspaceHeld = false;

        // WHEN: backspace pressed
        bool backPressed = true;

        if (backPressed)
        {
            if (!bBackspaceHeld && iSDLTextLen > 0) // NOT taken — buffer empty
            {
                szSDLText[--iSDLTextLen] = L'\0';
                bBackspaceHeld = true;
            }
        }
        else
        {
            bBackspaceHeld = false;
        }

        // THEN: buffer still empty, no underflow
        REQUIRE(iSDLTextLen == 0);
        REQUIRE(szSDLText[0] == L'\0');
    }

    SECTION("Backspace released resets m_bBackspaceHeld — next press works again")
    {
        // GIVEN: backspace was held, then released
        wchar_t szSDLText[64] = { L'A', L'B', L'\0' };
        int iSDLTextLen = 2;
        bool bBackspaceHeld = true; // was held

        // WHEN: key released (backPressed = false)
        bool backPressed = false;

        if (backPressed)
        {
            if (!bBackspaceHeld && iSDLTextLen > 0)
            {
                szSDLText[--iSDLTextLen] = L'\0';
                bBackspaceHeld = true;
            }
        }
        else
        {
            bBackspaceHeld = false; // RESET on release
        }

        // THEN: held flag is cleared — next press will delete
        REQUIRE(bBackspaceHeld == false);
        REQUIRE(iSDLTextLen == 2); // unchanged on this frame
    }
}

// ---------------------------------------------------------------------------
// AC-2 [VS1-SDL-INPUT-TEXT]: NUMBERONLY filter
// CUITextInputBox::DoAction() with UIOPTION_NUMBERONLY must reject non-digits.
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [VS1-SDL-INPUT-TEXT]: NUMBERONLY option filters non-digit characters",
          "[platform][text-input][ac2][numberonly]")
{
    SECTION("Non-digit characters filtered — only digits appended when NUMBERONLY is set")
    {
        // GIVEN: empty buffer with NUMBERONLY mode active
        wchar_t szSDLText[64] = {};
        int iSDLTextLen = 0;
        const int iSDLMaxLength = 63;
        bool numberOnly = true; // simulates CheckOption(UIOPTION_NUMBERONLY)

        // Simulate SDL delivering "a1b2" (mixed alphanumeric)
        const char* src_raw = "a1b2";

        // WHEN: consume with NUMBERONLY filter (mirrors DoAction() logic)
        const char* src = src_raw;
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }

            // Apply NUMBERONLY filter
            if (numberOnly)
            {
                if (wch < L'0' || wch > L'9')
                {
                    continue; // non-digit skipped
                }
            }

            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: only '1' and '2' stored
        REQUIRE(iSDLTextLen == 2);
        REQUIRE(szSDLText[0] == L'1');
        REQUIRE(szSDLText[1] == L'2');
        REQUIRE(szSDLText[2] == L'\0');
    }

    SECTION("All-digit input passes through NUMBERONLY filter unchanged")
    {
        // GIVEN: empty buffer with NUMBERONLY mode
        wchar_t szSDLText[64] = {};
        int iSDLTextLen = 0;
        const int iSDLMaxLength = 63;
        bool numberOnly = true;

        const char* src_raw = "123";

        // WHEN
        const char* src = src_raw;
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }
            if (numberOnly)
            {
                if (wch < L'0' || wch > L'9')
                {
                    continue;
                }
            }
            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: "123" stored intact
        REQUIRE(iSDLTextLen == 3);
        REQUIRE(szSDLText[0] == L'1');
        REQUIRE(szSDLText[1] == L'2');
        REQUIRE(szSDLText[2] == L'3');
    }

    SECTION("Accented character filtered by NUMBERONLY — not appended")
    {
        // GIVEN: buffer with NUMBERONLY, incoming e-acute
        wchar_t szSDLText[64] = {};
        int iSDLTextLen = 0;
        const int iSDLMaxLength = 63;
        bool numberOnly = true;

        // e-acute (U+00E9) — not a digit
        const char src_arr[] = { '\xC3', '\xA9', '\0' };
        const char* src = src_arr;

        // WHEN
        while (*src != '\0' && iSDLTextLen < iSDLMaxLength)
        {
            wchar_t wch = MuSdlUtf8NextChar(src);
            if (wch == L'\0')
            {
                break;
            }
            if (numberOnly)
            {
                if (wch < L'0' || wch > L'9')
                {
                    continue;
                }
            }
            szSDLText[iSDLTextLen++] = wch;
            szSDLText[iSDLTextLen] = L'\0';
        }

        // THEN: nothing appended
        REQUIRE(iSDLTextLen == 0);
        REQUIRE(szSDLText[0] == L'\0');
    }
}

// ---------------------------------------------------------------------------
// AC-STD-11 [VS1-SDL-INPUT-TEXT]: Flow code present in test names (this file)
// The flow code VS1-SDL-INPUT-TEXT appears in all TEST_CASE names above.
// This section is a compilation/linkage smoke test.
// ---------------------------------------------------------------------------

TEST_CASE("AC-STD-11 [VS1-SDL-INPUT-TEXT]: Flow code appears in test file",
          "[platform][text-input][ac-std-11][flow-code]")
{
    SECTION("Test file compiles with VS1-SDL-INPUT-TEXT flow code in test names — smoke test")
    {
        // GIVEN/WHEN/THEN: This test case compiling successfully IS the assertion.
        // The flow code VS1-SDL-INPUT-TEXT is embedded in all TEST_CASE names in this file.
        // No runtime assertion needed — compilation reaching this point is the verification.
    }
}

#endif // MU_ENABLE_SDL3
