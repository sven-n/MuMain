if(NOT INPUT OR NOT OUTPUT)
  message(FATAL_ERROR "INPUT and OUTPUT variables must be defined")
endif()

if(NOT EXISTS "${INPUT}")
  message(FATAL_ERROR "Input template file not found: ${INPUT}")
endif()

if(NOT EXISTS "${OUTPUT}")
  file(COPY_FILE "${INPUT}" "${OUTPUT}")
  message(STATUS "Seeded ${OUTPUT} from ${INPUT}")
endif()
