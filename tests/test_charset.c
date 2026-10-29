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
    static const uint8_t gbk_chinese[] = {0xd6u, 0xd0u, 0xceu, 0xc4u};
    static const uint8_t gb18030_emoji[] = {0x94u, 0x39u, 0xfcu, 0x36u};
    static const uint8_t shift_jis_japanese[] = {0x93u, 0xfau, 0x96u, 0x7bu};
    static const uint8_t euc_jp_japanese[] = {0xc6u, 0xfcu, 0xcbu, 0xdcu};
    static const uint8_t euc_kr_korean[] = {0xc7u, 0xd1u, 0xb1u, 0xb9u};
    static const uint8_t cp1251_cyrillic[] = {
        0xcfu, 0xf0u, 0xe8u, 0xe2u, 0xe5u, 0xf2u
    };
    static const uint8_t mac_roman_cafe[] = {'C', 'a', 'f', 0x8eu};
    static const uint8_t chinese_utf8[] = {
        0xe4u, 0xb8u, 0xadu, 0xe6u, 0x96u, 0x87u
    };
    static const uint8_t emoji_utf8[] = {0xf0u, 0x9fu, 0x98u, 0x80u};
    static const uint8_t japanese_utf8[] = {
        0xe6u, 0x97u, 0xa5u, 0xe6u, 0x9cu, 0xac
    };
    static const uint8_t korean_utf8[] = {
        0xedu, 0x95u, 0x9cu, 0xeau, 0xb5u, 0xadu
    };
    static const uint8_t cyrillic_utf8[] = {
        0xd0u, 0x9fu, 0xd1u, 0x80u, 0xd0u, 0xb8u,
        0xd0u, 0xb2u, 0xd0u, 0xb5u, 0xd1u, 0x82u
    };
    static const uint8_t cafe_utf8[] = {'C', 'a', 'f', 0xc3u, 0xa9u};
    static const uint8_t truncated_gbk[] = {0xd6u};
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

    CHECK(qw_charset_convert("CP936", "UTF-8", gbk_chinese,
        sizeof(gbk_chinese), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(chinese_utf8));
    CHECK(memcmp(output, chinese_utf8, sizeof(chinese_utf8)) == 0);
    CHECK(qw_charset_convert("UTF-8", "GBK", chinese_utf8,
        sizeof(chinese_utf8), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(gbk_chinese));
    CHECK(memcmp(output, gbk_chinese, sizeof(gbk_chinese)) == 0);
    CHECK(qw_charset_convert("GB18030", "UTF-8", gb18030_emoji,
        sizeof(gb18030_emoji), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(emoji_utf8));
    CHECK(memcmp(output, emoji_utf8, sizeof(emoji_utf8)) == 0);
    CHECK(qw_charset_convert("Shift-JIS", "UTF-8", shift_jis_japanese,
        sizeof(shift_jis_japanese), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(japanese_utf8));
    CHECK(memcmp(output, japanese_utf8, sizeof(japanese_utf8)) == 0);
    CHECK(qw_charset_convert("EUC-JP", "UTF-8", euc_jp_japanese,
        sizeof(euc_jp_japanese), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(japanese_utf8));
    CHECK(memcmp(output, japanese_utf8, sizeof(japanese_utf8)) == 0);
    CHECK(qw_charset_convert("CP949", "UTF-8", euc_kr_korean,
        sizeof(euc_kr_korean), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(korean_utf8));
    CHECK(memcmp(output, korean_utf8, sizeof(korean_utf8)) == 0);
    CHECK(qw_charset_convert("Windows-1251", "UTF-8", cp1251_cyrillic,
        sizeof(cp1251_cyrillic), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(cyrillic_utf8));
    CHECK(memcmp(output, cyrillic_utf8, sizeof(cyrillic_utf8)) == 0);
    CHECK(qw_charset_convert("Macintosh", "UTF-8", mac_roman_cafe,
        sizeof(mac_roman_cafe), output, sizeof(output), &output_size) == QW_CHARSET_OK);
    CHECK(output_size == sizeof(cafe_utf8));
    CHECK(memcmp(output, cafe_utf8, sizeof(cafe_utf8)) == 0);

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
    CHECK(qw_charset_convert("GBK", "UTF-8", truncated_gbk,
        sizeof(truncated_gbk), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_INVALID_SEQUENCE);
    CHECK(qw_charset_convert("EUC-TW", "UTF-8", gbk_chinese,
        sizeof(gbk_chinese), output, sizeof(output), &output_size) ==
        QW_CHARSET_ERR_UNSUPPORTED_ENCODING);
    memset(output, 0xcc, sizeof(output));
    CHECK(qw_charset_convert("GBK", "UTF-8", gbk_chinese,
        sizeof(gbk_chinese), output, 2u, &output_size) ==
        QW_CHARSET_ERR_BUFFER_TOO_SMALL);
    CHECK(output_size == sizeof(chinese_utf8));
    for (i = 0u; i < sizeof(output); ++i) CHECK(output[i] == 0xccu);
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
