#include <quirewise/charset.h>

#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s at line %d\n", #condition, __LINE__); \
        return 1; \
    } \
} while (0)

int main(void)
{
    static const uint8_t windows_1252[] = {'C', 'a', 'f', 0xe9u, ' ', 0x80u};
    static const uint8_t expected_utf8[] = {
        'C', 'a', 'f', 0xc3u, 0xa9u, ' ', 0xe2u, 0x82u, 0xacu
    };
    static const uint8_t utf16le[] = {
        0x41u, 0x00u, 0xacu, 0x20u, 0x3du, 0xd8u, 0x00u, 0xdeu
    };
    static const uint8_t utf16be[] = {
        0x00u, 0x41u, 0x20u, 0xacu, 0xd8u, 0x3du, 0xdeu, 0x00u
    };
    static const uint8_t expected_unicode_utf8[] = {
        0x41u, 0xe2u, 0x82u, 0xacu, 0xf0u, 0x9fu, 0x98u, 0x80u
    };
    static const uint8_t malformed_utf8[] = {0xc0u, 0xafu};
    static const uint8_t malformed_utf16[] = {0x00u, 0xd8u};
    static const uint8_t invalid_utf32[] = {0x00u, 0x00u, 0x11u, 0x00u};
    uint8_t output[32];
    size_t output_size = 0u;
    size_t i;

    CHECK(qw_charset_abi_version() == 0x00010000u);
    CHECK(qw_charset_convert("Windows-1252", "UTF_8", windows_1252,
        sizeof(windows_1252), NULL, 0u, &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(expected_utf8));
    memset(output, 0xcc, sizeof(output));
    CHECK(qw_charset_convert("windows1252", "utf8", windows_1252,
        sizeof(windows_1252), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(expected_utf8));
    CHECK(memcmp(output, expected_utf8, sizeof(expected_utf8)) == 0);

    CHECK(qw_charset_convert("utf-16le", "utf-8", utf16le, sizeof(utf16le),
        output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(expected_unicode_utf8));
    CHECK(memcmp(output, expected_unicode_utf8, sizeof(expected_unicode_utf8)) == 0);
    CHECK(qw_charset_convert("UTF-8", "UTF-16BE", expected_unicode_utf8,
        sizeof(expected_unicode_utf8), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(utf16be));
    CHECK(memcmp(output, utf16be, sizeof(utf16be)) == 0);
    CHECK(qw_charset_convert("UTF-16BE", "UTF-8", utf16be, sizeof(utf16be),
        output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(expected_unicode_utf8));
    CHECK(memcmp(output, expected_unicode_utf8, sizeof(expected_unicode_utf8)) == 0);
    CHECK(qw_charset_convert("UTF-8", "Windows-1252", expected_utf8,
        sizeof(expected_utf8), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(windows_1252));
    CHECK(memcmp(output, windows_1252, sizeof(windows_1252)) == 0);

    memset(output, 0xcc, sizeof(output));
    CHECK(qw_charset_convert("UTF-8", "ASCII", expected_utf8,
        sizeof(expected_utf8), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_UNREPRESENTABLE);
    for (i = 0u; i < sizeof(output); ++i) CHECK(output[i] == 0xccu);
    CHECK(qw_charset_convert("UTF-8", "UTF-8", malformed_utf8,
        sizeof(malformed_utf8), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_INVALID_SEQUENCE);
    CHECK(qw_charset_convert("UTF-16LE", "UTF-8", malformed_utf16,
        sizeof(malformed_utf16), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_INVALID_SEQUENCE);
    CHECK(qw_charset_convert("UTF-32LE", "UTF-8", invalid_utf32,
        sizeof(invalid_utf32), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_INVALID_SEQUENCE);
    CHECK(qw_charset_convert("not-an-encoding", "UTF-8", expected_utf8,
        sizeof(expected_utf8), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_UNSUPPORTED_ENCODING);
    CHECK(qw_charset_convert("UTF-8", "UTF-8", expected_utf8,
        sizeof(expected_utf8), output, 2u, &output_size) ==
        QW_CHARSET_ERR_BUFFER_TOO_SMALL);
    CHECK(output_size == sizeof(expected_utf8));
    for (i = 0u; i < sizeof(output); ++i) CHECK(output[i] == 0xccu);
    CHECK(qw_charset_convert("UTF-8", "UTF-8", NULL, 0u,
        NULL, 0u, &output_size) == QW_CHARSET_OK);
    CHECK(output_size == 0u);

    puts("PASS: charset aliases, Windows-1252, UTF-16, strict errors, and sizing");
    return 0;
}
