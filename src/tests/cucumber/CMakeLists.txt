file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/behave.ini CONTENT
        "[behave.userdata]\
\n\
antares-solver = $<TARGET_FILE:antares-solver>\
\n\
ts-generator = $<TARGET_FILE:antares-ts-generator>\
\n\
resources-path = ${CMAKE_SOURCE_DIR}/tests/resources/Antares_Simulator_Tests_NR"
        CONDITION $<STREQUAL:$<CONFIG>,${CMAKE_BUILD_TYPE}>)