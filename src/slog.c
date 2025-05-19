#include  "slog/slog.h"

#include <string.h> // For strcat, strcpy, strlen
#include <stdlib.h> // For malloc, free
#include <stdio.h>  // For FILE, fprintf, snprintf

// --- Global Logging System Configuration Definitions ---
format_log_event_func_t g_current_formatter = NULL;
log_sink_func_t g_current_sink = NULL;
FILE* g_default_log_stream = NULL; // Default to NULL, LOG_ERRORR will use stderr

// --- Logging System Initialization ---
void init_logging_system(format_log_event_func_t formatter, log_sink_func_t sink, FILE* default_stream) {
    g_current_formatter = formatter;
    g_current_sink = sink;
    g_default_log_stream = default_stream ? default_stream : stderr; // Ensure it's not NULL

    #ifdef LOGGING_LIB_DEBUG
    fprintf(g_default_log_stream, "[LoggingLib DEBUG] Logging system initialized. Formatter: %p, Sink: %p, Stream: %p\n",
            (void*)formatter, (void*)sink, (void*)g_default_log_stream);
    #endif
}

// --- Default Formatter Implementation (logfmt style) ---

/**
 * @brief Default logfmt style formatter.
 * Combines user message and KeyValuePairs into a logfmt string.
 * Example: user_message key1="value1" key2="value2"
 *
 * @param user_message The primary log message.
 * @param pairs Array of KeyValuePairs.
 * @param num_pairs Number of KeyValuePairs.
 * @return Dynamically allocated string in logfmt style, or NULL on failure.
 */
char* format_logfmt(const char* user_message, const KeyValuePair* pairs, int num_pairs) {
    // Calculate required buffer size
    size_t required_size = 0;
    if (user_message) {
        required_size += strlen(user_message);
    }

    for (int i = 0; i < num_pairs; ++i) {
        if (pairs[i].key && pairs[i].value) {
            required_size += strlen(pairs[i].key) + strlen(pairs[i].value) + 4; // key="value" (space, =, "", space/null)
        }
    }
    required_size += 1; // For null terminator
    if (num_pairs > 0 && user_message && strlen(user_message) > 0) {
        required_size += 1; // Space between user_message and first KVP
    }


    char* buffer = (char*)malloc(required_size);
    if (!buffer) {
        #ifdef LOGGING_LIB_DEBUG
        fprintf(g_default_log_stream ? g_default_log_stream : stderr,
                "[LoggingLib DEBUG] format_logfmt: Failed to allocate %zu bytes for log string.\n", required_size);
        #endif
        return NULL;
    }
    buffer[0] = '\0'; // Start with an empty string

    if (user_message && strlen(user_message) > 0) {
        strcpy(buffer, user_message);
    }

    for (int i = 0; i < num_pairs; ++i) {
        if (pairs[i].key && pairs[i].value) {
            if (strlen(buffer) > 0) { // Add space if buffer is not empty
                 // Check if previous char was not already a space (e.g. if user_message ended with one)
                if (buffer[strlen(buffer)-1] != ' ') {
                    strcat(buffer, " ");
                }
            }
            strcat(buffer, pairs[i].key);
            strcat(buffer, "=\"");
            // Basic escaping for quotes in value (simple version)
            // A more robust version would handle more special characters.
            char* value_ptr = pairs[i].value;
            while(*value_ptr) {
                if (*value_ptr == '"') {
                    strcat(buffer, "\\\""); // Escape quote
                } else if (*value_ptr == '\\') {
                    strcat(buffer, "\\\\"); // Escape backslash
                }
                else {
                    // Append char to buffer (strncat might be safer with buffer size checks)
                    char temp[2] = {*value_ptr, '\0'};
                    strcat(buffer, temp);
                }
                value_ptr++;
            }
            strcat(buffer, "\"");
        }
    }
    #ifdef LOGGING_LIB_DEBUG
    //fprintf(g_default_log_stream ? g_default_log_stream : stderr,
    //        "[LoggingLib DEBUG] format_logfmt: Formatted string (len %zu, cap %zu): %s\n", strlen(buffer), required_size, buffer);
    #endif
    return buffer;
}


// --- Default Sink Implementation ---

/**
 * @brief Default sink that writes the formatted log string to the given FILE stream.
 * Appends a newline character.
 *
 * @param formatted_log The null-terminated log string.
 * @param output_stream The FILE stream to write to.
 */
void sink_to_file_stream(const char* formatted_log, FILE* output_stream) {
    if (!output_stream) {
        // This should ideally not happen if init_logging_system ensures g_default_log_stream is valid
        output_stream = stderr;
    }
    fprintf(output_stream, "%s\n", formatted_log);
    fflush(output_stream); // Ensure it's written, especially for error logs
}

// --- Default Parser for Unknown Error Types ---

/**
 * @brief A parser for unknown error types, to be used as a default in
 * the user-defined LOG_ERROR_DISPATCH_TO_PARSER macro.
 *
 * @param error_details Pointer to the unknown error structure.
 * @param num_pairs_out Pointer to store the number of K-V pairs (will be 1 or 0).
 * @return A KeyValuePair array with a single entry describing the unknown type, or NULL on failure.
 */
KeyValuePair* parse_unknown_error(const void* error_details, int* num_pairs_out) {
    if (num_pairs_out) *num_pairs_out = 0;
    if (!error_details) return NULL;

    KeyValuePair* kvp_array = (KeyValuePair*)calloc(1, sizeof(KeyValuePair));
    if (!kvp_array) return NULL;

    kvp_array[0].key = strdup("unknown_error_type");
    if (!kvp_array[0].key) {
        free(kvp_array);
        return NULL;
    }

    char value_buffer[128]; // Reasonably sized buffer for pointer address
    snprintf(value_buffer, sizeof(value_buffer), "unhandled_type_at_address_%p", error_details);
    kvp_array[0].value = strdup(value_buffer);

    if (!kvp_array[0].value) {
        free(kvp_array[0].key);
        free(kvp_array);
        return NULL;
    }

    if (num_pairs_out) *num_pairs_out = 1;
    return kvp_array;
}
