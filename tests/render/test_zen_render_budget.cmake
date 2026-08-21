if(NOT DEFINED SOURCE_FILE)
  message(FATAL_ERROR "SOURCE_FILE is required")
endif()

file(READ "${SOURCE_FILE}" source)
string(FIND "${source}" "constexpr int MaxRenderedZenCoins = 12;" budget_definition)
string(FIND "${source}" "coinCount, MaxRenderedZenCoins" budget_use)

if(budget_definition EQUAL -1 OR budget_use EQUAL -1)
  message(FATAL_ERROR "RenderZen must cap cosmetic coin geometry at 12 coins per pile")
endif()
