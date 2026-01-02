//////////////////////////////////////////////////////////////////////////
//  GuildConstants.h
//  Central location for all guild-related constants to improve maintainability
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace GuildConstants
{
    // Guild Name and Mark Constants
    constexpr int GUILD_NAME_LENGTH = 8;
    constexpr int GUILD_NAME_BUFFER_SIZE = GUILD_NAME_LENGTH + 1;
    constexpr int GUILD_MARK_SIZE = 64;
    constexpr int GUILD_MARK_PIXELS = 8;  // 8x8 grid

    // Mark Processing Constants
    constexpr int MARK_NIBBLE_SHIFT = 4;
    constexpr BYTE MARK_NIBBLE_MASK = 0x0F;
    constexpr int INVALID_MARK_INDEX = -1;

    // Tab Indices
    enum class GuildTab : int
    {
        INFO = 0,      // Guild Info/History/Notice
        MEMBERS = 1,   // Guild Member List
        UNION = 2      // Union/Alliance
    };

    // Button Indices for Guild Info Window
    enum class GuildInfoButton : int
    {
        GUILD_OUT = 0,
        GET_POSITION = 1,
        FREE_POSITION = 2,
        GET_OUT = 3,
        UNION_CREATE = 4,
        UNION_OUT = 5,
        END = 6
    };

    // Guild Relationship Types
    enum class RelationshipType : BYTE
    {
        NONE = 0x00,
        UNION = 0x01,
        RIVAL = 0x02,
        UNION_MASTER = 0x04,
        RIVAL_UNION = 0x08
    };

    // Guild Relationship Request Types
    enum class RelationshipRequestType : BYTE
    {
        JOIN = 0x01,
        BREAK_OFF = 0x02
    };

    // UI Layout Constants
    namespace UILayout
    {
        // Main Guild Info Window
        constexpr int WINDOW_WIDTH = 190;
        constexpr int WINDOW_HEIGHT = 429;

        // Tab Button Layout
        constexpr int TAB_WIDTH = 56;
        constexpr int TAB_HEIGHT = 22;
        constexpr int TAB_START_X = 12;
        constexpr int TAB_START_Y = 68;
        constexpr int TAB_COUNT = 3;

        // Button Layout
        constexpr int BUTTON_WIDTH = 64;
        constexpr int BUTTON_HEIGHT = 29;

        // Scroll Bar
        constexpr int SCROLLBAR_WIDTH = 18;
        constexpr int SCROLLBAR_HEIGHT = 33;
        constexpr int SCROLL_RANGE_NOTICE = 49;
        constexpr int SCROLL_RANGE_MEMBERS = 200;

        // List Box Dimensions
        constexpr int NOTICE_BOX_WIDTH = 160;
        constexpr int NOTICE_BOX_HEIGHT = 80;
        constexpr int MEMBER_BOX_WIDTH = 160;
        constexpr int MEMBER_BOX_HEIGHT = 420;
        constexpr int UNION_BOX_WIDTH = 160;
        constexpr int UNION_BOX_HEIGHT = 80;

        // Guild Mark Display
        constexpr int MARK_DISPLAY_SIZE = 42;
        constexpr int MARK_RENDER_SIZE = 39;

        // Text Layout
        constexpr int TEXT_SPLIT_WIDTH = 110;
        constexpr int TEXT_MAX_LINES = 5;
    }

    // Guild Make Window Constants
    namespace MakeWindow
    {
        constexpr int MAX_GUILD_NAME = 8;
        constexpr int TEMP_TEXT_BUFFER_SIZE = 100;

        enum class State : int
        {
            INFO = 0,
            MARK = 1,
            RESULT_INFO = 2
        };

        enum class ButtonIndex : int
        {
            INFO_MAKE = 0,
            MARK_LNEXT = 1,
            MARK_RNEXT = 2,
            RESULTINFO_LNEXT = 3,
            RESULTINFO_RNEXT = 4,
            COUNT = 5
        };
    }

    // Guild Member Capacity Constants
    namespace Capacity
    {
        constexpr int BASE_CAPACITY_PER_10_LEVELS = 1;
        constexpr int DARK_LORD_BONUS_PER_10_CHARISMA = 1;
        constexpr int MAX_CAPACITY = 80;
    }

    // Color Constants (ARGB format)
    namespace Colors
    {
        constexpr DWORD YELLOW = 0xFFC8FF64;  // Guild name color (200, 255, 100, 255)
        constexpr DWORD WHITE = 0xFFFFFFFF;
        constexpr DWORD GRAY = 0xFF999999;
    }
}
