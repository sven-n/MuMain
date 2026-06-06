#pragma once

#ifdef _EDITOR

#include <sstream>
#include <string>

/**
 * Simple change tracking for data structures
 *
 * Provides easy-to-use functions for comparing old vs new data
 * and logging all changes.
 */

namespace ChangeTracker
{
    /**
     * Extract UTF-8 name from a struct with wchar_t Name field
     */
    template<typename TStruct>
    inline std::string GetNameUtf8(int index, const TStruct& data, int maxNameLen)
    {
        char nameBuf[256] = {0};
        int bufSize = (maxNameLen > 0 && maxNameLen < 256) ? maxNameLen : sizeof(nameBuf);
        int result = WideCharToMultiByte(CP_UTF8, 0, data.Name, -1, nameBuf, bufSize, NULL, NULL);
        if (result == 0)
        {
            return std::string("[UTF-8 conversion failed]");
        }
        return std::string(nameBuf);
    }

    /**
     * Compare any two values and log the difference
     */
    template<typename T>
    inline void CompareField(const char* fieldName, const T& oldVal, const T& newVal,
                            std::stringstream& changes, bool& hasChanged)
    {
        if (oldVal != newVal)
        {
            changes << "  " << fieldName << ": " << oldVal << " -> " << newVal << "\n";
            hasChanged = true;
        }
    }

    /**
     * Compare two bool values and log as true/false
     */
    inline void CompareField(const char* fieldName, const bool& oldVal, const bool& newVal,
                            std::stringstream& changes, bool& hasChanged)
    {
        if (oldVal != newVal)
        {
            changes << "  " << fieldName << ": "
                   << (oldVal ? "true" : "false") << " -> "
                   << (newVal ? "true" : "false") << "\n";
            hasChanged = true;
        }
    }

    /**
     * Compare two BYTE values and display as integers
     */
    inline void CompareField(const char* fieldName, const BYTE& oldVal, const BYTE& newVal,
                            std::stringstream& changes, bool& hasChanged)
    {
        if (oldVal != newVal)
        {
            changes << "  " << fieldName << ": "
                   << (int)oldVal << " -> " << (int)newVal << "\n";
            hasChanged = true;
        }
    }

    /**
     * Compare two wide-character strings
     */
    inline void CompareWideString(const char* fieldName, const wchar_t* oldVal, const wchar_t* newVal,
                                 std::stringstream& changes, bool& hasChanged, int maxLen)
    {
        bool stringsEqual = (maxLen > 0) ? (wcsncmp(oldVal, newVal, maxLen) == 0)
                                         : (wcscmp(oldVal, newVal) == 0);

        if (!stringsEqual)
        {
            char oldUtf8[256] = {0};
            char newUtf8[256] = {0};
            int charCount = (maxLen > 0) ? maxLen : -1;
            int result1 = WideCharToMultiByte(CP_UTF8, 0, oldVal, charCount, oldUtf8, sizeof(oldUtf8), NULL, NULL);
            int result2 = WideCharToMultiByte(CP_UTF8, 0, newVal, charCount, newUtf8, sizeof(newUtf8), NULL, NULL);

            if (result1 == 0 || result2 == 0)
            {
                changes << "  " << fieldName << ": [UTF-8 conversion failed]\n";
            }
            else
            {
                changes << "  " << fieldName << ": \""
                       << oldUtf8 << "\" -> \"" << newUtf8 << "\"\n";
            }
            hasChanged = true;
        }
    }
}

// ===========================================================================
// OLD MACRO-BASED COMPARISON (DEPRECATED - Use FieldMetadata.h instead!)
// ===========================================================================
// These macros are kept for backward compatibility only.
// New code should use CompareAllFieldsByMetadata() from FieldMetadata.h
// with C++20 constexpr field arrays instead.
//
// Example migration:
//   OLD: CompareAllFields(oldItem, newItem, changes, changed, MAX_ITEM_NAME, ITEM_FIELDS_SIMPLE, ITEM_FIELDS_ARRAYS);
//   NEW: CompareAllFieldsByMetadata(oldItem, newItem, ItemComparisonMetadata::SIMPLE_FIELDS, changes, changed);
//        CompareAllFieldsByMetadata(oldItem, newItem, ItemComparisonMetadata::ARRAY_FIELDS, changes, changed);
// ===========================================================================

#endif // _EDITOR
