# One-time comparison: every text the client used to fetch by legacy id
# (I18N::<Group>::Lookup) against the resx key that replaced it, in every
# locale. Removed together with the legacy ids. The game code it reaches
# needs the client globals of Winmain.cpp.
mu_add_test(
    NAME legacy_text_ids_tests
    SOURCES
        test_legacy_text_character.cpp
        test_legacy_text_items.cpp
        test_legacy_text_ui.cpp
        test_legacy_text_dialog.cpp
        ${CMAKE_SOURCE_DIR}/src/source/App/Platform/Windows/Winmain.cpp
    LINK_LIBS MuClient
)
