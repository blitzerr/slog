# src/CMakeLists.txt (Modified)
add_library(${SLOG_LIBRARY_NAME} # Use the variable here
        slog.c
        # other_source_files.c
)

target_include_directories(${SLOG_LIBRARY_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../inc"
)
# ... other target properties for ${SLOG_LIBRARY_NAME}
