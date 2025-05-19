#include  "slog/slog.h"
#include  "slog/err_gen.h"

#include <assert.h>
#include <stdio.h>

#include <ctype.h>  // For isprint

// Compares two strings character by character. If they don't match,
// it prints detailed information about the first difference found to stdout.
// Handles NULL input strings.
//
// Parameters:
//   s1 - The first string.
//   s2 - The second string.
//
// Returns:
//   0 if strings are identical (or both NULL).
//   1 if strings are different (or one is NULL and the other is not).
int compare_strings_and_print_diff(const char* s1, const char* s2) {
    // Handle NULL cases first
    if (s1 == NULL && s2 == NULL) {
        // Optionally print: printf("Both strings are NULL.\n");
        return 0; // Both NULL can be considered identical.
    }
    if (s1 == NULL) {
        printf("s1 is NULL, s2 is not NULL.\n");
        printf("  s2 (first 50 chars): \"%.50s%s\"\n", s2, (s2[50] ? "..." : ""));
        return 1;
    }
    if (s2 == NULL) {
        printf("s2 is NULL, s1 is not NULL.\n");
        printf("  s1 (first 50 chars): \"%.50s%s\"\n", s1, (s1[50] ? "..." : ""));
        return 1;
    }

    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            printf("Mismatch at index %d:\n", i);
            // Print characters, their ASCII values, and hex values.
            // Use '.' for non-printable characters in the '%c' part.
            printf("  s1[%d] = '%c' (ASCII: %3d, Hex: 0x%02X)\n", i,
                   isprint((unsigned char)s1[i]) ? s1[i] : '.',
                   (int)(unsigned char)s1[i], (unsigned char)s1[i]);
            printf("  s2[%d] = '%c' (ASCII: %3d, Hex: 0x%02X)\n", i,
                   isprint((unsigned char)s2[i]) ? s2[i] : '.',
                   (int)(unsigned char)s2[i], (unsigned char)s2[i]);

            // Configuration for context display
            int context_chars_before = 10;
            int context_chars_after = 10; // How many chars to show after the mismatch

            // Print context for s1
            printf("  Context s1: \"");
            int start_ctx = (i > context_chars_before) ? (i - context_chars_before) : 0;
            if (start_ctx > 0) printf("...");
            for (int k = start_ctx; k < i + context_chars_after && s1[k] != '\0'; ++k) {
                if (k == i) printf("[%c]", isprint((unsigned char)s1[k]) ? s1[k] : '.');
                else printf("%c", isprint((unsigned char)s1[k]) ? s1[k] : '.');
            }
            // Check if string continued beyond the printed context part
            if (s1[i + context_chars_after] != '\0') printf("...");
            printf("\"\n");

            // Print context for s2
            printf("  Context s2: \"");
            if (start_ctx > 0) printf("..."); // Re-use start_ctx calculated for s1
            for (int k = start_ctx; k < i + context_chars_after && s2[k] != '\0'; ++k) {
                if (k == i) printf("[%c]", isprint((unsigned char)s2[k]) ? s2[k] : '.');
                else printf("%c", isprint((unsigned char)s2[k]) ? s2[k] : '.');
            }
            if (s2[i + context_chars_after] != '\0') printf("...");
            printf("\"\n");
            return 1; // Mismatch found
        }
        i++;
    }

    // After the loop, one or both strings have reached their null terminator.
    // Check if they ended at the same position.
    if (s1[i] == '\0' && s2[i] == '\0') {
        // Both strings ended at the same time, and all characters up to this point matched.
        return 0; // Identical
    } else if (s1[i] == '\0') {
        // s1 ended, but s2 has more characters.
        printf("s1 ended at index %d (length %d), but s2 is longer.\n", i, i);
        printf("  s2 continues at index %d with '%c' (ASCII: %3d, Hex: 0x%02X)\n", i,
               isprint((unsigned char)s2[i]) ? s2[i] : '.',
               (int)(unsigned char)s2[i], (unsigned char)s2[i]);
        printf("  Remaining s2 (first 20 chars): \"");
        for(int k=i; k < i+20 && s2[k] != '\0'; ++k) {
            printf("%c", isprint((unsigned char)s2[k]) ? s2[k] : '.');
        }
        if (s2[i+20] != '\0') printf("...");
        printf("\"\n");
        return 1; // Different lengths
    } else { // s2[i] == '\0'
        // s2 ended, but s1 has more characters.
        printf("s2 ended at index %d (length %d), but s1 is longer.\n", i, i);
        printf("  s1 continues at index %d with '%c' (ASCII: %3d, Hex: 0x%02X)\n", i,
               isprint((unsigned char)s1[i]) ? s1[i] : '.',
               (int)(unsigned char)s1[i], (unsigned char)s1[i]);
        printf("  Remaining s1 (first 20 chars): \"");
        for(int k=i; k < i+20 && s1[k] != '\0'; ++k) {
            printf("%c", isprint((unsigned char)s1[k]) ? s1[k] : '.');
        }
        if (s1[i+20] != '\0') printf("...");
        printf("\"\n");
        return 1; // Different lengths
    }
}

#define POINT_FIELDS(X_PARAM) \
X_PARAM(int, x, PRIMITIVE, _NA_, "%d") \
X_PARAM(int, y, PRIMITIVE, _NA_, "%d")

DEFINE_STRUCT_AND_TO_STRING(Point, POINT_FIELDS);

#define LINE_FIELDS(X_PARAM) \
X_PARAM(Point, start, STRUCT, Point, NULL) \
X_PARAM(Point, end, STRUCT, Point, NULL) \
X_PARAM(const char*, label, PRIMITIVE, _NA_, "%s")

DEFINE_STRUCT_AND_TO_STRING(Line, LINE_FIELDS);

#define SPECIAL_FEATURE_FIELDS(X_PARAM) \
X_PARAM(int, feature_id, PRIMITIVE, _NA_, "%d") \
X_PARAM(const char*, feature_name, PRIMITIVE, _NA_, "%s")

DEFINE_STRUCT_ONLY(SpecialFeature, SPECIAL_FEATURE_FIELDS)
int SpecialFeature_to_str(char* buffer, size_t buffer_size, const SpecialFeature* s, const char* name_prefix) {
    if (s == NULL) {
        if (buffer_size > 0) buffer[0] = '\0';
        return 0;
    }

    char full_feature_prefix[256];
    if (name_prefix && name_prefix[0] != '\0') {
        snprintf(full_feature_prefix, sizeof(full_feature_prefix), "%s", name_prefix);
    } else {
        full_feature_prefix[0] = '\0';
    }

    char temp_buffer[512];
    int len1, len2;

    char id_name[300];
    snprintf(id_name, sizeof(id_name), "%s%sfeature_id",
             full_feature_prefix, (full_feature_prefix[0] == '\0' ? "" : "."));

    len1 = snprintf(temp_buffer, sizeof(temp_buffer), "%s=%d (custom_fmt)", id_name, s->feature_id);
    if (len1 < 0 || (size_t)len1 >= sizeof(temp_buffer)) return -1;

    char name_field_name[300];
    snprintf(name_field_name, sizeof(name_field_name), "%s%sfeature_name",
            full_feature_prefix, (full_feature_prefix[0] == '\0' ? "" : "."));

    const char* f_name = s->feature_name ? s->feature_name : "null";
    len2 = snprintf(temp_buffer + len1, sizeof(temp_buffer) - len1, " %s=\\\"%s\\\" (custom_fmt)", name_field_name, f_name);
    if (len2 < 0 || (size_t)len2 >= (sizeof(temp_buffer) - len1)) return -1;

    if ((size_t)(len1 + len2) >= buffer_size) {
        if (buffer_size > 0) buffer[buffer_size-1] = '\0';
        return -1;
    }
    strcpy(buffer, temp_buffer);
    return len1 + len2;
}

#define PRODUCT_FIELDS(X_PARAM) \
X_PARAM(int, product_sku, PRIMITIVE, _NA_, "%d") \
X_PARAM(SpecialFeature, main_feature, STRUCT, SpecialFeature, NULL) \
X_PARAM(const char*, product_name, PRIMITIVE, _NA_, "%s")

DEFINE_STRUCT_AND_TO_STRING(Product, PRODUCT_FIELDS)


void test_struct_to_str()
{
    Point p1 = {10, 20};
    Line l1 = {p1, {30, 40}, "MainLine"};

    SpecialFeature sf1 = {101, "SuperSpeed"};
    Product prod1 = {9001, sf1, "Awesome Gadget"};
    Product prod2 = {9002, {202, NULL}, "Basic Gadget"};

    char buffer[1024];

    printf("--- Testing Point (Auto-generated) ---\n");
    assert(25 ==  Point_to_str(buffer, sizeof(buffer), &p1, "mypoint"));
    assert(strcmp(buffer, "mypoint.x=10 mypoint.y=20") == 0);

    printf("\n--- Testing Line (Auto-generated, uses auto Point_to_string) ---\n");
    int len = Line_to_str(buffer, sizeof(buffer), &l1, "myline");

    assert(89 == len);
    assert(strcmp(buffer, "myline.start.x=10 myline.start.y=20 myline.end.x=30 myline.end.y=40 myline.label=MainLine") == 0);

    printf("\n--- Testing SpecialFeature (Custom _to_string directly) ---\n");
    len = SpecialFeature_to_str(buffer, sizeof(buffer), &sf1, NULL);
    assert(len == 68);
    printf("SpecialFeature sf1 (NULL prefix, len %d): '%s'\n", len, buffer);

    const char* expected = "feature_id=101 (custom_fmt) feature_name=\"SuperSpeed\" (custom_fmt)";

    len = SpecialFeature_to_str(buffer, sizeof(buffer), &sf1, "feat");
    assert(len == 78);
    assert(strcmp(buffer, "feat.feature_id=101 (custom_fmt) feat.feature_name=\"SuperSpeed\" (custom_fmt)") == 0);

    printf("\n--- Testing Product (Auto-generated, uses custom SpecialFeature_to_string) ---\n");
    len = Product_to_str(buffer, sizeof(buffer), &prod1, NULL);
    assert(len == 139);
    //assert(strcmp(buffer, "product_sku=9001 main_feature.feature_id=101 (custom_fmt) main_feature.feature_name=\"SuperSpeed\" (custom_fmt) product_name=Awesome Gadget") == 0);
    printf("Product prod1 (len %d): %s\n", len, buffer);

    len = Product_to_str(buffer, sizeof(buffer), &prod2, "item");
    assert(len == 151);
    assert(strcmp(buffer, "item.product_sku=9002 item.main_feature.feature_id=202 (custom_fmt) item.main_feature.feature_name=\"null\" (custom_fmt) item.product_name=Basic Gadget") == 0);

    printf("\n--- Testing Buffer Safety with custom function (small buffer) ---\n");
    SpecialFeature sf_long_name = {777, "ThisIsAVeryLongFeatureNameDesignedToCauseOverflowInSmallBuffers"};
    char small_buffer[50];
    len = SpecialFeature_to_str(small_buffer, sizeof(small_buffer), &sf_long_name, "test");
    if (len < 0) {
        printf("Overflow test (custom SpecialFeature_to_string): Correctly indicated error/truncation.\n");
        printf("Small buffer content: \"%s\"\n", small_buffer);
    } else {
        assert(false);
    }
}

int main()
{
    test_struct_to_str();
    return 0;
}
