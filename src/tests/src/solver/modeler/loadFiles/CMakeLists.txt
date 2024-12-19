add_executable(load-modeler-files
               testLoadModelerFiles.cpp)

target_link_libraries(load-modeler-files
        PRIVATE
        Boost::unit_test_framework
        Antares::loadModelerFiles
        test_utils_unit
)

set_target_properties(load-modeler-files PROPERTIES FOLDER Unit-tests/test-writer)

add_test(NAME load-modeler-files COMMAND load-modeler-files)
set_tests_properties(load-modeler-files PROPERTIES LABELS unit)
