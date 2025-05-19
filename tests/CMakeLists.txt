# tests/CMakeLists.txt (Example)
add_executable(slog_tests
        test_slog.c
        # other_test_files.c
)

# Use the variable for linking
target_link_libraries(slog_tests PRIVATE ${SLOG_LIBRARY_NAME})

add_test(NAME SlogUnitTests COMMAND slog_tests)
