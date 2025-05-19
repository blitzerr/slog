#pragma once

typedef enum {
    PRIMITIVE,
    STRUCT
} FieldKind;

#define _XMACRO_FIELD_DEFINER(ctype, cname, ckind, struct_type_token, primitive_format_spec) ctype cname;

#define _DEFINE_STRUCT_INTERNAL(STRUCT_NAME, FIELDS_XMACRO_LIST) \
    typedef struct STRUCT_NAME \
    { \
        FIELDS_XMACRO_LIST(_XMACRO_FIELD_DEFINER) \
    }STRUCT_NAME;

#define _XMACRO_FIELD_TO_STRING_LOGIC(ctype, cname, ckind, struct_type_token, primitive_format_spec) \
    do { \
        /* Add a space separator if this is not the first field being printed for this struct instance */ \
        if (!first_field_in_struct) { \
            if (offset < buffer_size) { \
                buffer[offset++] = ' '; \
            } else { \
                /* Not enough space for the separator, indicate error/truncation */ \
                if (buffer_size > 0) {buffer[buffer_size-1] = '\0';} /* Ensure null termination */ \
                return -1; \
            } \
        } \
        first_field_in_struct = 0; /* Subsequent fields will get a preceding space */ \
        \
        /* Buffer to construct the full field name (e.g., "outer.inner.fieldname") */ \
        char field_full_name[256]; \
        if (name_prefix && name_prefix[0] != '\0') { \
            snprintf(field_full_name, sizeof(field_full_name), "%s.%s", name_prefix, #cname); \
        } else { \
            snprintf(field_full_name, sizeof(field_full_name), "%s", #cname); \
        } \
        \
        if (ckind == PRIMITIVE) { \
            const char* fmt_spec = (const char*)primitive_format_spec; /* printf format specifier */ \
            char format_string_buffer[512]; /* Buffer for the complete format string, e.g., "fieldname=%s" */ \
                /* Format: fieldname=value for other primitive types */ \
                snprintf(format_string_buffer, sizeof(format_string_buffer), "%s=%s", field_full_name, fmt_spec); \
                res = snprintf(buffer + offset, (buffer_size > offset ? buffer_size - offset : 0), format_string_buffer, s->cname); \
            /* Check for snprintf errors or buffer truncation */ \
            if (res < 0 || (size_t)res >= (buffer_size > offset ? buffer_size - offset : 0) ) { \
                if (buffer_size > 0 && offset < buffer_size) buffer[offset] = '\0'; \
                else if (buffer_size > 0) buffer[buffer_size-1] = '\0'; /* Force null termination if full */ \
                return -1; \
            } \
            offset += res; /* Advance offset by number of characters written */ \
        } else if (ckind == STRUCT) { \
            /* For nested structs, recursively call its _to_string function. */ \
            /* cextra is the literal type name of the nested struct (e.g., Point). */ \
            /* The call becomes: Point_to_string(...) or UserProvidedStruct_to_string(...) */ \
            res = struct_type_token##_to_str(buffer + offset, (buffer_size > offset ? buffer_size - offset : 0), &s->cname, field_full_name); \
            if (res < 0) { /* Error propagated from nested call */ \
                if (buffer_size > 0 && offset < buffer_size) buffer[offset] = '\0'; \
                else if (buffer_size > 0) buffer[buffer_size-1] = '\0'; \
                return -1; \
            } \
            offset += res; \
        } \
    } while(0);

#define _DEFINE_TO_STR_INTERNAL(STRUCT_NAME, FIELDS_XMACRO_LIST) \
    int STRUCT_NAME##_to_str(char* buffer, size_t buffer_size, const struct STRUCT_NAME* s, const char* name_prefix) { \
        if (s == NULL) {\
            if (buffer_size > 0) { \
                buffer[0] = '\0'; \
                return 0; \
            } \
        } \
        int offset = 0; /* Current position in the output buffer */ \
        int res;        /* Result of snprintf calls */ \
        int first_field_in_struct = 1; /* Flag to manage spacing between fields */ \
        FIELDS_XMACRO_LIST(_XMACRO_FIELD_TO_STRING_LOGIC) \
        if (buffer_size > 0 && offset < buffer_size) { \
            buffer[offset] = '\0'; \
        } else if (buffer_size > 0 && offset >= buffer_size) { \
            /* If we exactly filled or overflowed the buffer, the last char must be null. */ \
            /* snprintf usually handles this, but this is a safeguard. */ \
            buffer[buffer_size-1] = '\0'; \
        } \
        return offset; \
    }

#define DEFINE_STRUCT_AND_TO_STRING(STRUCT_NAME, FIELDS_XMACRO_LIST) \
    _DEFINE_STRUCT_INTERNAL(STRUCT_NAME, FIELDS_XMACRO_LIST) \
    _DEFINE_TO_STR_INTERNAL(STRUCT_NAME, FIELDS_XMACRO_LIST)

#define DEFINE_STRUCT_ONLY(STRUCT_NAME, FIELDS_XMACRO_LIST) \
    _DEFINE_STRUCT_INTERNAL(STRUCT_NAME, FIELDS_XMACRO_LIST)

#define _NA_ _NA_TOKEN_UNUSED_

int _NA__to_str(char* buffer, size_t buffer_size, const void* s, const char* name_prefix)
{
    return 0;
}
