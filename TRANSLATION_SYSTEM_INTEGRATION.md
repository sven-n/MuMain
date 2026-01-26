# Translation System Integration

## Overview

The MU Online client now includes a comprehensive translation system that supports internationalization (i18n) across different components of the application. The system is designed with **conditional compilation** to ensure that only necessary translation domains are included in each build configuration.

## Architecture

### Translation Domains

The system organizes translations into three separate domains:

1. **Game Domain** (`i18n::Domain::Game`)
   - **Availability**: Always compiled (both Debug and Release builds)
   - **Purpose**: In-game text, UI labels, messages, dialogs, tooltips
   - **Use Case**: Player-facing content that needs localization in production
   - **Example Keys**: `"button_login"`, `"error_connection_failed"`, `"tooltip_inventory"`

2. **Editor Domain** (`i18n::Domain::Editor`)
   - **Availability**: Debug/Editor builds only (`#ifdef _EDITOR`)
   - **Purpose**: MU Editor UI, tool labels, editor-specific messages
   - **Use Case**: Development tools and in-game editor interface
   - **Example Keys**: `"editor_title"`, `"log_exported_s6e3"`, `"button_save_item"`

3. **Metadata Domain** (`i18n::Domain::Metadata`)
   - **Availability**: Debug/Editor builds only (`#ifdef _EDITOR`)
   - **Purpose**: Item field names, attribute labels, technical metadata
   - **Use Case**: Human-readable labels for data structures in the editor
   - **Example Keys**: `"field_Durability"`, `"field_Level"`, `"field_TwoHand"`

### Conditional Compilation Strategy

The translation system uses preprocessor directives to exclude editor/metadata functionality from release builds:

```cpp
// In i18n.h
enum class Domain {
#ifdef _EDITOR
    Editor,     // Only available in debug/editor builds
    Metadata,   // Only available in debug/editor builds
#endif
    Game        // Always available (both debug and release)
};
```

**Benefits**:
- **Reduced Binary Size**: Release builds don't include editor/metadata translation maps
- **Zero Overhead**: No runtime cost for unused features in production
- **Clear Separation**: Compiler enforces proper usage at build time

## File Structure

```
MuMain/
├── src/
│   ├── Translation/
│   │   ├── i18n.h              # Translation system header (domain-aware)
│   │   └── i18n.cpp            # Translation system implementation
│   ├── GameData/
│   │   └── ItemData/
│   │       └── ItemFieldMetadata.h  # Uses metadata translations (#ifdef _EDITOR)
│   ├── MuEditor/
│   │   └── Core/
│   │       └── MuEditorCore.cpp     # Loads Editor and Metadata translations (#ifdef _EDITOR)
│   └── bin/
│       └── Translations/
│           ├── en/
│           │   ├── game.json       # Game translations (always loaded)
│           │   ├── editor.json     # Editor translations (debug only)
│           │   └── metadata.json   # Metadata translations (debug only)
│           └── [other locales]/   # de, es, id, pl, pt, ru, tl, uk
```

## Translation File Format

Translation files use simple JSON format with key-value pairs:

```json
{
  "key_name": "Translated text",
  "another_key": "Text with {0} and {1} placeholders",
  "multiline_key": "Line 1\nLine 2\nLine 3"
}
```

**Supported Features**:
- Simple key-value mappings
- Escape sequences: `\n` (newline), `\t` (tab), `\"` (quote)
- Placeholder formatting: `{0}`, `{1}`, etc. for dynamic content

## API Usage

### Game Translations (Always Available)

```cpp
// Available in both Debug and Release builds
#include "Translation/i18n.h"

// Simple translation
const char* text = i18n::TranslateGame("button_login", "Login");
// Or use macro
const char* text2 = GAME_TEXT("button_login");

// Check if translation exists
if (i18n::Translator::GetInstance().HasTranslation(i18n::Domain::Game, "key")) {
    // ...
}
```

### Editor Translations (Debug Only)

```cpp
// Only available when _EDITOR is defined
#ifdef _EDITOR
#include "Translation/i18n.h"

// Simple translation
const char* label = i18n::TranslateEditor("editor_title", "MU Item Editor");
// Or use macro
const char* label2 = EDITOR_TEXT("button_export");

// Format with placeholders
std::string msg = i18n::FormatEditor("log_exported_s6e3", {filename});
// Result: "Exported to {filename} (S6E3 format)"
#endif
```

### Metadata Translations (Debug Only)

```cpp
// Only available when _EDITOR is defined
#ifdef _EDITOR
#include "Translation/i18n.h"

// Translate field names
const char* fieldName = i18n::TranslateMetadata("field_Durability", "Durability");
// Or use macro with fallback
const char* fieldName2 = META_TEXT("field_Level", "Level");

// Check and translate
std::string key = "field_" + std::string(fieldName);
if (i18n::HasTranslation(i18n::Domain::Metadata, key.c_str())) {
    return i18n::TranslateMetadata(key.c_str(), fieldName);
}
#endif
```

### Loading Translations

```cpp
// Typically done during initialization (MuEditorCore.cpp)
i18n::Translator& translator = i18n::Translator::GetInstance();

// Game translations - always loaded
bool gameLoaded = translator.LoadTranslations(i18n::Domain::Game,
    L"Translations/en/game.json");

#ifdef _EDITOR
// Editor and metadata translations - only in debug builds
bool editorLoaded = translator.LoadTranslations(i18n::Domain::Editor,
    L"Translations/en/editor.json");
bool metadataLoaded = translator.LoadTranslations(i18n::Domain::Metadata,
    L"Translations/en/metadata.json");
#endif
```

## Convenience Macros

| Macro | Domain | Availability | Usage |
|-------|--------|--------------|-------|
| `GAME_TEXT(key)` | Game | Always | `GAME_TEXT("button_login")` |
| `EDITOR_TEXT(key)` | Editor | Debug only | `EDITOR_TEXT("editor_title")` |
| `META_TEXT(key, fallback)` | Metadata | Debug only | `META_TEXT("field_Level", "Level")` |

## Build Configuration Impact

### Debug Builds (`Global Debug`)
- **Preprocessor**: `_EDITOR` is defined
- **Translation Domains**: All three (Game, Editor, Metadata)
- **Binary Size**: Larger due to full translation support
- **Use Case**: Development, testing, in-game editor

### Release Builds (`Global Release`)
- **Preprocessor**: `_EDITOR` is NOT defined
- **Translation Domains**: Game only
- **Binary Size**: Smaller, optimized for production
- **Use Case**: Public client distribution

## Adding New Translations

### 1. Add Translation Keys to JSON Files

**src/bin/Translations/en/game.json**:
```json
{
  "new_button": "Click Me",
  "new_message": "Welcome, {0}!"
}
```

**src/bin/Translations/en/editor.json** (Debug only):
```json
{
  "new_tool": "Advanced Tool",
  "log_action": "Action completed: {0}"
}
```

**src/bin/Translations/en/metadata.json** (Debug only):
```json
{
  "field_NewAttribute": "New Attribute"
}
```

### 2. Use in Code

```cpp
// Game translation (always available)
const char* buttonText = GAME_TEXT("new_button");
std::string msg = i18n::Translator::GetInstance().Format(
    i18n::Domain::Game, "new_message", {playerName});

#ifdef _EDITOR
// Editor translation (debug only)
const char* toolName = EDITOR_TEXT("new_tool");
std::string log = i18n::FormatEditor("log_action", {"export"});

// Metadata translation (debug only)
const char* attrName = META_TEXT("field_NewAttribute", "NewAttribute");
#endif
```

## Implementation Details

### Singleton Pattern

The `Translator` class uses the singleton pattern to ensure a single global instance:

```cpp
i18n::Translator& translator = i18n::Translator::GetInstance();
```

### Translation Maps

Internally, the system maintains separate `std::map<std::string, std::string>` for each domain:

```cpp
// In i18n.h
class Translator {
private:
#ifdef _EDITOR
    std::map<std::string, std::string> m_editorTranslations;   // Debug only
    std::map<std::string, std::string> m_metadataTranslations; // Debug only
#endif
    std::map<std::string, std::string> m_gameTranslations;      // Always available
};
```

### Fallback Mechanism

All translation functions support fallback values:

```cpp
const char* Translate(Domain domain, const char* key, const char* fallback = nullptr);
```

- If key is found: returns translated text
- If key is not found and fallback is provided: returns fallback
- If key is not found and no fallback: returns the key itself

## Performance Considerations

1. **Translation Lookup**: O(log n) using `std::map::find()`
2. **Memory Overhead**:
   - Release: Only game translations (~minimal overhead)
   - Debug: All three domains (~moderate overhead for development)
3. **Load Time**: JSON files are parsed once during initialization
4. **Runtime Cost**: Zero-cost abstraction in release builds (inline functions)
