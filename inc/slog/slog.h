#pragma once

#include <stdio.h>   // For snprintf, FILE, fprintf, vsnprintf
#include <string.h>  // For strlen, strdup, strcat, strncat
#include <stdlib.h>  // For malloc, free, calloc
#include <stdarg.h>  // For va_list, va_start, va_end
#include <stdbool.h> // For bool type

// --- Configuration ---
// Define this to enable extensive internal logging/debugging for the logging library itself.
// #define LOGGING_LIB_DEBUG

// --- Core Data Structures ---

/**
 * @brief Represents a key-value pair for structured logging.
 * Both key and value are expected to be dynamically allocated strings.
 */
typedef struct {
    char* key;
    char* value;
} KeyValuePair;

// --- Function Pointer Typedefs ---

/**
 * @brief Function pointer type for an error parser.
 *
 * Takes a pointer to an error details structure and returns a dynamically
 * allocated array of KeyValuePair. The number of pairs is returned via num_pairs_out.
 * The caller is responsible for freeing the array and the strings within each pair
 * using free_kv_pairs().
 *
 * @param error_details Pointer to the specific error structure instance.
 * @param num_pairs_out Pointer to an integer to store the number of K-V pairs generated.
 * @return Dynamically allocated array of KeyValuePair, or NULL on failure or if no pairs.
 */
typedef KeyValuePair* (*parse_error_func_t)(const void* error_details, int* num_pairs_out);

/**
 * @brief Function pointer type for a log event formatter.
 *
 * Takes a user-provided message, an array of KeyValuePairs, and their count.
 * It should format these into a single, dynamically allocated string.
 * The caller is responsible for freeing the returned string.
 *
 * @param user_message The primary log message (e.g., from printf_format_string).
 * @param pairs Array of KeyValuePairs representing structured error data. Can be NULL if num_pairs is 0.
 * @param num_pairs Number of KeyValuePairs. Can be 0.
 * @return A dynamically allocated string representing the fully formatted log entry, or NULL on failure.
 */
typedef char* (*format_log_event_func_t)(const char* user_message, const KeyValuePair* pairs, int num_pairs);

/**
 * @brief Function pointer type for a log sink.
 *
 * Takes the final formatted log string and an output stream (or other target context)
 * and writes/sends the log message.
 *
 * @param formatted_log The null-terminated string to log.
 * @param output_stream The FILE stream to write to (e.g., stderr, a file).
 * This could be generalized to void* context for other sink types.
 */
typedef void (*log_sink_func_t)(const char* formatted_log, FILE* output_stream);

// --- Global Logging System Configuration (declared here, defined in .c file) ---

/**
 * @brief Global pointer to the current formatter function.
 * Must be set by init_logging_system().
 */
extern format_log_event_func_t g_current_formatter;

/**
 * @brief Global pointer to the current sink function.
 * Must be set by init_logging_system().
 */
extern log_sink_func_t g_current_sink;

/**
 * @brief Global pointer to the default FILE stream for logging.
 * Can be overridden. Defaults to stderr if not set before first log.
 */
extern FILE* g_default_log_stream;

// --- Logging System Initialization ---

/**
 * @brief Initializes the logging system with a specific formatter, sink, and default stream.
 *
 * @param formatter The formatter function to use.
 * @param sink The sink function to use.
 * @param default_stream The default FILE stream (e.g., stderr, or a log file).
 * If NULL, stderr will be used.
 */
void init_logging_system(format_log_event_func_t formatter, log_sink_func_t sink, FILE* default_stream);

// --- Utility Functions ---

/**
 * @brief Frees the memory allocated for an array of KeyValuePairs and their contents.
 *
 * @param pairs The array of KeyValuePairs to free.
 * @param num_pairs The number of pairs in the array.
 */
static inline void free_kv_pairs(KeyValuePair* pairs, int num_pairs) {
    if (!pairs) {
        return;
    }
    for (int i = 0; i < num_pairs; ++i) {
        free(pairs[i].key); // Free the duplicated key string
        free(pairs[i].value); // Free the formatted value string
    }
    free(pairs); // Free the array itself
}

// --- Core Macros for Defining Error Structs and Parsers ---

/**
 * @brief Internal helper macro to define a struct member.
 * (Same as user's original)
 */
#define _ERROR_FIELD_DEFINE_MEMBER(type, name, printf_fmt, doc_str) type name;

// /**
//  * @brief Defines an error structure and its specific parser function.
//  */
// #define DEFINE_ERROR_TYPE(StructName, FIELDS_XMACRO) \
//     typedef struct StructName { \
//         FIELDS_XMACRO(_ERROR_FIELD_DEFINE_MEMBER) \
//     } StructName; \
//     \
//     _Pragma("GCC diagnostic push") \
//     _Pragma("GCC diagnostic ignored \"-Wunused-function\"") \
//     _Pragma("clang diagnostic push") \
//     _Pragma("clang diagnostic ignored \"-Wunused-function\"") \
//     static KeyValuePair* parse_##StructName(const StructName* error_instance, int* num_pairs_out) { \
//         if (!error_instance) { \
//             if (num_pairs_out) *num_pairs_out = 0; \
//             return NULL; \
//         } \
//         \
//         /* 1. Count fields */ \
//         /* Removed _Pragmas around this #define/#undef block for _ERROR_FIELD_COUNTER_ACTION */ \
//         #define _ERROR_FIELD_COUNTER_ACTION(type, name, printf_fmt, doc_str) +1 \
//         int num_fields = 0 FIELDS_XMACRO(_ERROR_FIELD_COUNTER_ACTION); \
//         #undef _ERROR_FIELD_COUNTER_ACTION \
//         \
//         if (num_fields == 0) { \
//             if (num_pairs_out) *num_pairs_out = 0; \
//             return NULL; \
//         } \
//         \
//         KeyValuePair* pairs = (KeyValuePair*)calloc((size_t)num_fields, sizeof(KeyValuePair)); \
//         if (!pairs) { \
//             if (num_pairs_out) *num_pairs_out = 0; \
//             return NULL; \
//         } \
//         \
//         int current_idx = 0; \
//         bool parse_success = true; \
//         \
//         /* 2. Populate KeyValuePairs */ \
//         /* Removed _Pragmas around this #define/#undef block for _ERROR_FIELD_POPULATE_KV_ACTION */ \
//         #define _ERROR_FIELD_POPULATE_KV_ACTION(type, name, printf_fmt, doc_str) \
//         if (parse_success) { \
//             pairs[current_idx].key = strdup(#name); \
//             if (!pairs[current_idx].key) { \
//                 parse_success = false; \
//             } else { \
//                 int value_len = snprintf(NULL, 0, printf_fmt, error_instance->name); \
//                 if (value_len < 0) { \
//                     parse_success = false; \
//                 } else { \
//                     pairs[current_idx].value = (char*)malloc((size_t)value_len + 1); \
//                     if (!pairs[current_idx].value) { \
//                         parse_success = false; \
//                     } else { \
//                         snprintf(pairs[current_idx].value, (size_t)value_len + 1, printf_fmt, error_instance->name); \
//                         current_idx++; \
//                     } \
//                 } \
//             } \
//         } \
//         \
//         FIELDS_XMACRO(_ERROR_FIELD_POPULATE_KV_ACTION) \
//         #undef _ERROR_FIELD_POPULATE_KV_ACTION \
//         \
//         if (!parse_success) { \
//             for (int i = 0; i < num_fields; ++i) { \
//                  free(pairs[i].key); pairs[i].key = NULL; \
//                  free(pairs[i].value); pairs[i].value = NULL; \
//             } \
//             free(pairs); \
//             if (num_pairs_out) *num_pairs_out = 0; \
//             return NULL; \
//         } \
//         \
//         if (num_pairs_out) *num_pairs_out = current_idx; \
//         return pairs; \
//     } \
//     _Pragma("GCC diagnostic pop") \
//     _Pragma("clang diagnostic pop")

// --- Generic Logging Macro ---

/**
 * @brief USER ACTION REQUIRED: Define this macro in your own code (e.g., main.c or a specific error_types.c).
 * This macro uses C11 _Generic to dispatch an error_details_ptr to the correct
 * type-specific parser function generated by DEFINE_ERROR_TYPE.
 *
 * Example Definition:
 * ```c
 * // Forward declare your parsers or include headers where DEFINE_ERROR_TYPE is used.
 * // extern KeyValuePair* parse_FileError(const FileError* error_instance, int* num_pairs_out);
 * // extern KeyValuePair* parse_NetworkError(const NetworkError* error_instance, int* num_pairs_out);
 * // extern KeyValuePair* parse_unknown_error(const void* error_details, int* num_pairs_out); // A fallback
 *
 * #define LOG_ERROR_DISPATCH_TO_PARSER(error_details_ptr, num_pairs_out_ptr) \
 * _Generic((error_details_ptr), \
 * const FileError*: parse_FileError((const FileError*)(error_details_ptr), (num_pairs_out_ptr)), \
 * const NetworkError*: parse_NetworkError((const NetworkError*)(error_details_ptr), (num_pairs_out_ptr)), \
 * default: parse_unknown_error((error_details_ptr), (num_pairs_out_ptr)) \
 * )
 * ```
 * The `default` case should point to a generic parser that handles unknown types,
 * possibly by returning a single K-V pair like {"error_type": "unknown", "address": "%p"}.
 */
#define LOG_ERROR_DISPATCH_TO_PARSER(error_details_ptr, num_pairs_out_ptr) \
    generic_error_parser_placeholder(error_details_ptr, num_pairs_out_ptr)
    // IMPORTANT: The line above is a non-functional placeholder!
    // The user MUST #undef this and #define their own version with actual _Generic dispatch.
    // We provide a dummy function to make the header technically compile if the user forgets.
    // See error_logging_impl.c for an example of `parse_unknown_error`.


// Placeholder function for LOG_ERROR_DISPATCH_TO_PARSER if user doesn't define it.
// This is primarily to allow the LOG_ERRORR macro to compile.
// It's not intended for actual use without the user defining their dispatch.
static inline KeyValuePair* generic_error_parser_placeholder(const void* error_details, int* num_pairs_out) {
    (void)error_details; // Suppress unused parameter warning
    #ifdef LOGGING_LIB_DEBUG
    fprintf(stderr, "[LoggingLib DEBUG] WARNING: generic_error_parser_placeholder called. "
                    "User should define LOG_ERROR_DISPATCH_TO_PARSER.\n");
    #endif
    if (num_pairs_out) *num_pairs_out = 0;
    // Optionally, create a single KVP indicating an unknown type if error_details is not NULL
    if (error_details) {
        KeyValuePair* kvp = (KeyValuePair*)calloc(1, sizeof(KeyValuePair));
        if (kvp) {
            kvp->key = strdup("logging_error");
            char val_buf[128];
            snprintf(val_buf, sizeof(val_buf), "LOG_ERROR_DISPATCH_TO_PARSER not defined by user for type at %p", error_details);
            kvp->value = strdup(val_buf);
            if (kvp->key && kvp->value) {
                 if (num_pairs_out) *num_pairs_out = 1;
                 return kvp;
            } else {
                free(kvp->key); free(kvp->value); free(kvp);
            }
        }
    }
    return NULL;
}


/**
 * @brief Main logging macro.
 *
 * Logs an error with structured details and a formatted message.
 *
 * @param error_details_ptr Pointer to the specific error structure instance (e.g., &my_file_error).
 * Can be NULL if no structured details are available.
 * @param user_fmt printf-style format string for the main log message.
 * @param ... Variadic arguments for the user_fmt.
 */
#define LOG_ERRORR(error_details_ptr, user_fmt, ...) \
    do { \
        const void* __local_err_ptr = (error_details_ptr); \
        KeyValuePair* __kv_pairs = NULL; \
        int __num_pairs = 0; \
        char* __final_log_message = NULL; \
        FILE* __current_log_stream = g_default_log_stream ? g_default_log_stream : stderr; \
        \
        /* Ensure logging system is initialized */ \
        if (!g_current_formatter || !g_current_sink) { \
            fprintf(__current_log_stream, "[LOG_ERRORR Critical] Logging system not initialized. User message: "); \
            fprintf(__current_log_stream, (user_fmt), ##__VA_ARGS__); \
            fprintf(__current_log_stream, "\n"); \
            break; /* Exit do-while */ \
        } \
        \
        /* 1. Parse error_details into K-V pairs if provided */ \
        if (__local_err_ptr) { \
            __kv_pairs = LOG_ERROR_DISPATCH_TO_PARSER(__local_err_ptr, &__num_pairs); \
            /* LOG_ERROR_DISPATCH_TO_PARSER's default case should handle unknown types. */ \
            /* It might return NULL or a specific K-V pair like {"error": "unknown_type"}. */ \
        } \
        \
        /* 2. Format the user's variadic message part */ \
        char __user_message_buffer[1024]; /* Static buffer for user message; consider dynamic for very long messages. */ \
        va_list __args; \
        va_start(__args, user_fmt); \
        int __user_msg_len = vsnprintf(__user_message_buffer, sizeof(__user_message_buffer), (user_fmt), __args); \
        va_end(__args); \
        \
        if (__user_msg_len < 0) { \
            /* vsnprintf error */ \
            snprintf(__user_message_buffer, sizeof(__user_message_buffer), "LOG_ERRORR: vsnprintf failed for user message. Original fmt: %s", (user_fmt)); \
        } else if ((size_t)__user_msg_len >= sizeof(__user_message_buffer)) { \
            /* User message was truncated. Append a truncation indicator if space allows. */ \
            const char* __trunc_indicator = "...(msg_truncated)"; \
            size_t __remaining_space = sizeof(__user_message_buffer) - strlen(__user_message_buffer) - 1; \
            if (strlen(__trunc_indicator) <= __remaining_space) { \
                strcat(__user_message_buffer, __trunc_indicator); \
            } \
        } \
        \
        /* 3. Call the formatter with user message and K-V pairs */ \
        __final_log_message = g_current_formatter(__user_message_buffer, __kv_pairs, __num_pairs); \
        \
        /* Free K-V pairs now that formatter is (supposedly) done with them */ \
        if (__kv_pairs) { \
            free_kv_pairs(__kv_pairs, __num_pairs); \
            __kv_pairs = NULL; /* Avoid double free if loop breaks and re-enters somehow (not an issue with do-while(0)) */ \
        } \
        \
        if (!__final_log_message) { \
            /* Formatter failed. Log raw user message and a warning. */ \
            fprintf(__current_log_stream, "[LOG_ERRORR Warning] Formatter failed. Raw user message: %s\n", __user_message_buffer); \
            if (__local_err_ptr && !__kv_pairs && __num_pairs == 0) { /* Check if parser might have failed */ \
                 fprintf(__current_log_stream, "[LOG_ERRORR Warning] Parsing of error details might have also failed for ptr %p.\n", __local_err_ptr); \
            } \
            break; /* Exit do-while */ \
        } \
        \
        /* 4. Call the sink */ \
        g_current_sink(__final_log_message, __current_log_stream); \
        free(__final_log_message); \
        __final_log_message = NULL; /* Avoid double free */ \
        \
    } while(0)

// --- Original Helper Macros (can be kept for other uses or direct printing if needed) ---

/**
 * @brief Internal helper macro to log a struct member directly to console (printf).
 * This is for scenarios where direct printing is preferred over string formatting.
 * (From user's original code)
 */
#define _ERROR_FIELD_LOG_MEMBER_DIRECT_PRINT(type, name, printf_fmt, doc_str) \
    printf("  %s = " printf_fmt " \t/* Doc: %s */\n", #name, error_instance->name, doc_str);


/**
 * @brief A fallback logging function for error types not explicitly handled by _Generic.
 * This version prints directly to console.
 * (From user's original code, can be used for debugging or simple direct logging)
 */
static inline void log_unknown_error_type_handler_direct_print(const void* ptr, const char* type_name_guess) {
    FILE* __current_log_stream = g_default_log_stream ? g_default_log_stream : stderr;
    if (!ptr) {
        fprintf(__current_log_stream, "--- Logging Error: Unknown Type ---\n");
        fprintf(__current_log_stream, "  Error: Attempted to log a NULL pointer of an unknown or unhandled type.\n");
        fprintf(__current_log_stream, "--- End Error: Unknown Type ---\n\n");
        return;
    }
    const char* type_str = type_name_guess ? type_name_guess : "unknown type";
    fprintf(__current_log_stream, "--- Logging Error: %s (at %p) ---\n", type_str, ptr);
    fprintf(__current_log_stream, "  Error: Instance of unrecognized/unhandled error type '%s'. No specific structured parser found or direct print called.\n", type_str);
    fprintf(__current_log_stream, "  Tip: Ensure this error type is listed in your LOG_ERROR_DISPATCH_TO_PARSER macro for structured logging.\n");
    fprintf(__current_log_stream, "--- End Error: %s ---\n\n", type_str);
}
