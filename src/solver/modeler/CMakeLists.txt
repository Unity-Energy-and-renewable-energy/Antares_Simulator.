add_subdirectory(api)
add_subdirectory(ortoolsImpl)
add_subdirectory(loadFiles)
add_subdirectory(parameters)


#
# Resource file for Windows
#
if (WIN32)
    file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/win32/modeler.o")
    FILE(COPY "${CMAKE_CURRENT_SOURCE_DIR}/win32/modeler.ico" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/win32/")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/win32/modeler.rc.cmake"
            "${CMAKE_CURRENT_SOURCE_DIR}/win32/modeler.rc")

    if (MINGW)
        # resource compilation for mingw
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/win32/modeler.o"
                COMMAND windres.exe "-I${CMAKE_CURRENT_SOURCE_DIR}"
                "-i${CMAKE_CURRENT_SOURCE_DIR}/win32/modeler.rc"
                -o "${CMAKE_CURRENT_BINARY_DIR}/win32/modeler.o")
        set(SRCS ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/win32/modeler.o")
    else ()
        set(SRCS ${SRCS} "win32/modeler.rc")
    endif ()
endif ()

OMESSAGE("  :: modeler")

set(exec_name "antares-modeler")

add_library(modeler-lib INTERFACE
        ${SRCS}
)

add_executable(antares-modeler
        main.cpp
        ${SRCS}
)

set_target_properties(antares-modeler PROPERTIES OUTPUT_NAME ${exec_name})

target_link_libraries(modeler-lib
        INTERFACE
        Antares::loadModelerFiles
        Antares::modelerParameters
)

target_link_libraries(antares-modeler
        PRIVATE
        modeler-lib
)

import_std_libs(antares-modeler)
executable_strip(antares-modeler)

copy_dependency(sirius_solver antares-modeler)

install(TARGETS antares-modeler EXPORT antares-modeler DESTINATION bin)

INSTALL(EXPORT antares-modeler
        FILE antares-modelerConfig.cmake
        DESTINATION cmake
)
