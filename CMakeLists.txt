cmake_minimum_required(VERSION 3.31)
project(slog C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Define the library name as a variable
set(SLOG_LIBRARY_NAME "slog_lib")

# Option to enable/disable building tests and examples
option(SLOG_BUILD_TESTING "Build the slog tests" ON)
option(SLOG_BUILD_EXAMPLES "Build example application" ON)

add_subdirectory(src) # src/CMakeLists.txt will define the ${SLOG_LIBRARY_NAME} target

if(SLOG_BUILD_TESTING)
    enable_testing() # Enables CTest
    add_subdirectory(tests) # tests/CMakeLists.txt will link against ${SLOG_LIBRARY_NAME}
endif()

if(SLOG_BUILD_EXAMPLES)
    add_executable(slog_main_app examples/main.c)
    # Use the variable for linking
    target_link_libraries(slog_main_app PRIVATE ${SLOG_LIBRARY_NAME})
endif()

include(GNUInstallDirs)
# Use the variable for installing the target
install(TARGETS ${SLOG_LIBRARY_NAME}
        EXPORT SlogTargets # For others to use find_package(SlogProject)
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY inc/slog/ # Assuming your public headers are in inc/slog/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/slog
        FILES_MATCHING PATTERN "*.h"
)
