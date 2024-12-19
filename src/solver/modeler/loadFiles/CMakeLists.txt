set(SOURCES
        readSystem.cpp
        readLibraries.cpp
)

# Create the library
add_library(loadFiles STATIC ${SOURCES})
add_library(Antares::loadFiles ALIAS loadFiles)

# Specify include directories
target_include_directories(loadFiles
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Link dependencies (if any)
target_link_libraries(loadFiles
        PUBLIC
        Antares::antares-study-system-model
        PRIVATE
        io
        Antares::systemParser
        Antares::modelParser
        Antares::modelConverter
)

install(DIRECTORY include/antares
        DESTINATION "include"
)
