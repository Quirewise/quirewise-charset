#include <quirewise/charset.h>

#include <string.h>

typedef enum QwCharsetEncoding {
    QW_ENCODING_UTF8,
    QW_ENCODING_UTF16LE,
    QW_ENCODING_UTF16BE,
    QW_ENCODING_UTF32LE,
    QW_ENCODING_UTF32BE,
    QW_ENCODING_ASCII,
    QW_ENCODING_LATIN1,
    QW_ENCODING_WINDOWS1252,
    QW_ENCODING_CP1250,
    QW_ENCODING_CP1251,
    QW_ENCODING_CP1253,
    QW_ENCODING_CP1254,
    QW_ENCODING_CP1255,
    QW_ENCODING_CP1256,
    QW_ENCODING_CP1257,
    QW_ENCODING_CP1258,
    QW_ENCODING_ISO88597,
    QW_ENCODING_KOI8R,
    QW_ENCODING_KOI8U,
    QW_ENCODING_CP437,
    QW_ENCODING_CP850,
    QW_ENCODING_CP852,
    QW_ENCODING_CP866,
    QW_ENCODING_CP874,
    QW_ENCODING_MACROMAN,
    QW_ENCODING_GBK,
    QW_ENCODING_GB2312,
    QW_ENCODING_BIG5,
    QW_ENCODING_CP950,
    QW_ENCODING_CP932,
    QW_ENCODING_CP949,
    QW_ENCODING_EUCJP,
    QW_ENCODING_EUCKR,
    QW_ENCODING_GB18030
} QwCharsetEncoding;

typedef struct QwCharsetMapEntry {
    uint32_t key;
    uint32_t value;
    uint8_t width;
} QwCharsetMapEntry;

typedef struct QwCharsetGb18030Range {
    uint32_t first_pointer;
    uint32_t last_pointer;
    uint32_t first_codepoint;
} QwCharsetGb18030Range;

#include "charset_tables.h"

static const uint16_t windows_1252_high[32] = {
    0x20acu, 0x0081u, 0x201au, 0x0192u, 0x201eu, 0x2026u, 0x2020u, 0x2021u,
    0x02c6u, 0x2030u, 0x0160u, 0x2039u, 0x0152u, 0x008du, 0x017du, 0x008fu,
    0x0090u, 0x2018u, 0x2019u, 0x201cu, 0x201du, 0x2022u, 0x2013u, 0x2014u,
    0x02dcu, 0x2122u, 0x0161u, 0x203au, 0x0153u, 0x009du, 0x017eu, 0x0178u
};

static int charset_name(const char *name, QwCharsetEncoding *encoding)
{
    char normalized[48];
    size_t source = 0u;
    size_t target = 0u;
    if (!name || !encoding) return 0;
    while (name[source]) {
        unsigned char ch = (unsigned char)name[source++];
        if (ch == '-' || ch == '_' || ch == ' ' || ch == '.') continue;
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9')) || target + 1u >= sizeof(normalized)) {
            return 0;
        }
        if (ch >= 'A' && ch <= 'Z') ch = (unsigned char)(ch + ('a' - 'A'));
        normalized[target++] = (char)ch;
    }
    if (!target) return 0;
    normalized[target] = '\0';

    if (strcmp(normalized, "utf8") == 0) *encoding = QW_ENCODING_UTF8;
    else if (strcmp(normalized, "utf16le") == 0) *encoding = QW_ENCODING_UTF16LE;
    else if (strcmp(normalized, "utf16be") == 0) *encoding = QW_ENCODING_UTF16BE;
    else if (strcmp(normalized, "utf32le") == 0) *encoding = QW_ENCODING_UTF32LE;
    else if (strcmp(normalized, "utf32be") == 0) *encoding = QW_ENCODING_UTF32BE;
    else if (strcmp(normalized, "ascii") == 0 ||
             strcmp(normalized, "usascii") == 0 ||
             strcmp(normalized, "ansix341968") == 0) *encoding = QW_ENCODING_ASCII;
    else if (strcmp(normalized, "iso88591") == 0 ||
             strcmp(normalized, "latin1") == 0 ||
             strcmp(normalized, "latin") == 0 || strcmp(normalized, "l1") == 0 ||
             strcmp(normalized, "cp819") == 0) *encoding = QW_ENCODING_LATIN1;
    else if (strcmp(normalized, "windows1252") == 0 ||
             strcmp(normalized, "cp1252") == 0 ||
             strcmp(normalized, "xcp1252") == 0) *encoding = QW_ENCODING_WINDOWS1252;
    else if (strcmp(normalized, "windows1250") == 0 ||
             strcmp(normalized, "cp1250") == 0 ||
             strcmp(normalized, "xcp1250") == 0) *encoding = QW_ENCODING_CP1250;
    else if (strcmp(normalized, "windows1251") == 0 ||
             strcmp(normalized, "cp1251") == 0 ||
             strcmp(normalized, "xcp1251") == 0) *encoding = QW_ENCODING_CP1251;
    else if (strcmp(normalized, "windows1253") == 0 ||
             strcmp(normalized, "cp1253") == 0) *encoding = QW_ENCODING_CP1253;
    else if (strcmp(normalized, "windows1254") == 0 ||
             strcmp(normalized, "cp1254") == 0) *encoding = QW_ENCODING_CP1254;
    else if (strcmp(normalized, "windows1255") == 0 ||
             strcmp(normalized, "cp1255") == 0) *encoding = QW_ENCODING_CP1255;
    else if (strcmp(normalized, "windows1256") == 0 ||
             strcmp(normalized, "cp1256") == 0) *encoding = QW_ENCODING_CP1256;
    else if (strcmp(normalized, "windows1257") == 0 ||
             strcmp(normalized, "cp1257") == 0) *encoding = QW_ENCODING_CP1257;
    else if (strcmp(normalized, "windows1258") == 0 ||
             strcmp(normalized, "cp1258") == 0) *encoding = QW_ENCODING_CP1258;
    else if (strcmp(normalized, "iso88597") == 0 ||
             strcmp(normalized, "greek") == 0 ||
             strcmp(normalized, "greek8") == 0) *encoding = QW_ENCODING_ISO88597;
    else if (strcmp(normalized, "koi8r") == 0 ||
             strcmp(normalized, "koi") == 0 ||
             strcmp(normalized, "koi8") == 0 ||
             strcmp(normalized, "koi8ru") == 0) *encoding = QW_ENCODING_KOI8R;
    else if (strcmp(normalized, "koi8u") == 0) *encoding = QW_ENCODING_KOI8U;
    else if (strcmp(normalized, "cp437") == 0 ||
             strcmp(normalized, "ibm437") == 0) *encoding = QW_ENCODING_CP437;
    else if (strcmp(normalized, "cp850") == 0 ||
             strcmp(normalized, "ibm850") == 0) *encoding = QW_ENCODING_CP850;
    else if (strcmp(normalized, "cp852") == 0 ||
             strcmp(normalized, "ibm852") == 0) *encoding = QW_ENCODING_CP852;
    else if (strcmp(normalized, "cp866") == 0 ||
             strcmp(normalized, "ibm866") == 0) *encoding = QW_ENCODING_CP866;
    else if (strcmp(normalized, "cp874") == 0 ||
             strcmp(normalized, "windows874") == 0) *encoding = QW_ENCODING_CP874;
    else if (strcmp(normalized, "macroman") == 0 ||
             strcmp(normalized, "macintosh") == 0 ||
             strcmp(normalized, "xmacroman") == 0) *encoding = QW_ENCODING_MACROMAN;
    else if (strcmp(normalized, "gbk") == 0 ||
             strcmp(normalized, "cp936") == 0 ||
             strcmp(normalized, "windows936") == 0 ||
             strcmp(normalized, "xgbk") == 0) *encoding = QW_ENCODING_GBK;
    else if (strcmp(normalized, "gb2312") == 0 ||
             strcmp(normalized, "euccn") == 0 ||
             strcmp(normalized, "chinese") == 0 ||
             strcmp(normalized, "csgb2312") == 0 ||
             strcmp(normalized, "csiso58gb231280") == 0 ||
             strcmp(normalized, "isoir58") == 0) *encoding = QW_ENCODING_GB2312;
    else if (strcmp(normalized, "big5") == 0 ||
             strcmp(normalized, "cnbig5") == 0 ||
             strcmp(normalized, "csbig5") == 0 ||
             strcmp(normalized, "xxbig5") == 0 ||
             strcmp(normalized, "big5hkscs") == 0) *encoding = QW_ENCODING_BIG5;
    else if (strcmp(normalized, "cp950") == 0 ||
             strcmp(normalized, "windows950") == 0) *encoding = QW_ENCODING_CP950;
    else if (strcmp(normalized, "shiftjis") == 0 ||
             strcmp(normalized, "sjis") == 0 ||
             strcmp(normalized, "mskanji") == 0 ||
             strcmp(normalized, "cp932") == 0 ||
             strcmp(normalized, "ms932") == 0 ||
             strcmp(normalized, "csshiftjis") == 0 ||
             strcmp(normalized, "windows31j") == 0 ||
             strcmp(normalized, "xsjis") == 0) *encoding = QW_ENCODING_CP932;
    else if (strcmp(normalized, "cp949") == 0 ||
             strcmp(normalized, "windows949") == 0) *encoding = QW_ENCODING_CP949;
    else if (strcmp(normalized, "eucjp") == 0 ||
             strcmp(normalized, "xeucjp") == 0 ||
             strcmp(normalized, "cseucpkdfmtjapanese") == 0) *encoding = QW_ENCODING_EUCJP;
    else if (strcmp(normalized, "euckr") == 0 ||
             strcmp(normalized, "cseuckr") == 0 ||
             strcmp(normalized, "csksc56011987") == 0 ||
             strcmp(normalized, "isoir149") == 0 ||
             strcmp(normalized, "korean") == 0 ||
             strcmp(normalized, "ksc5601") == 0) *encoding = QW_ENCODING_EUCKR;
    else if (strcmp(normalized, "gb18030") == 0 ||
             strcmp(normalized, "gb180302000") == 0) *encoding = QW_ENCODING_GB18030;
    else return 0;
    return 1;
}

static int valid_codepoint(uint32_t codepoint)
{
    return codepoint <= 0x10ffffu &&
           !(codepoint >= 0xd800u && codepoint <= 0xdfffu);
}

static uint16_t read_u16(const uint8_t *data, int big_endian)
{
    return big_endian
        ? (uint16_t)(((uint16_t)data[0] << 8) | data[1])
        : (uint16_t)(((uint16_t)data[1] << 8) | data[0]);
}

static uint32_t read_u32(const uint8_t *data, int big_endian)
{
    if (big_endian) {
        return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
               ((uint32_t)data[2] << 8) | (uint32_t)data[3];
    }
    return ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) |
           ((uint32_t)data[1] << 8) | (uint32_t)data[0];
}

#define QW_MAP_CASE(encoding_name, table_name, count_name) \
    case QW_ENCODING_##encoding_name: \
        *decode = qw_charset_##table_name##_decode; \
        *decode_count = QW_CHARSET_##count_name##_DECODE_COUNT; \
        *encode = qw_charset_##table_name##_encode; \
        *encode_count = QW_CHARSET_##count_name##_ENCODE_COUNT; \
        return 1

static int encoding_tables(QwCharsetEncoding encoding,
                           const QwCharsetMapEntry **decode,
                           size_t *decode_count,
                           const QwCharsetMapEntry **encode,
                           size_t *encode_count)
{
    switch (encoding) {
    QW_MAP_CASE(CP1250, cp1250, CP1250);
    QW_MAP_CASE(CP1251, cp1251, CP1251);
    QW_MAP_CASE(CP1253, cp1253, CP1253);
    QW_MAP_CASE(CP1254, cp1254, CP1254);
    QW_MAP_CASE(CP1255, cp1255, CP1255);
    QW_MAP_CASE(CP1256, cp1256, CP1256);
    QW_MAP_CASE(CP1257, cp1257, CP1257);
    QW_MAP_CASE(CP1258, cp1258, CP1258);
    QW_MAP_CASE(ISO88597, iso8859_7, ISO8859_7);
    QW_MAP_CASE(KOI8R, koi8_r, KOI8_R);
    QW_MAP_CASE(KOI8U, koi8_u, KOI8_U);
    QW_MAP_CASE(CP437, cp437, CP437);
    QW_MAP_CASE(CP850, cp850, CP850);
    QW_MAP_CASE(CP852, cp852, CP852);
    QW_MAP_CASE(CP866, cp866, CP866);
    QW_MAP_CASE(CP874, cp874, CP874);
    QW_MAP_CASE(MACROMAN, mac_roman, MAC_ROMAN);
    QW_MAP_CASE(GBK, gbk, GBK);
    QW_MAP_CASE(GB2312, gb2312, GB2312);
    QW_MAP_CASE(BIG5, big5, BIG5);
    QW_MAP_CASE(CP950, cp950, CP950);
    QW_MAP_CASE(CP932, cp932, CP932);
    QW_MAP_CASE(CP949, cp949, CP949);
    QW_MAP_CASE(EUCJP, euc_jp, EUC_JP);
    QW_MAP_CASE(EUCKR, euc_kr, EUC_KR);
    QW_MAP_CASE(GB18030, gb18030, GB18030);
    default:
        return 0;
    }
}

#undef QW_MAP_CASE

static const QwCharsetMapEntry *find_map_entry(
    const QwCharsetMapEntry *entries, size_t count, uint32_t key)
{
    size_t low = 0u;
    size_t high = count;
    while (low < high) {
        size_t middle = low + (high - low) / 2u;
        if (entries[middle].key < key) low = middle + 1u;
        else high = middle;
    }
    if (low < count && entries[low].key == key) return entries + low;
    return NULL;
}

static const QwCharsetGb18030Range *find_gb18030_pointer(uint32_t pointer)
{
    size_t i;
    for (i = 0u; i < QW_CHARSET_GB18030_RANGE_COUNT; ++i) {
        const QwCharsetGb18030Range *range = qw_charset_gb18030_ranges + i;
        if (pointer >= range->first_pointer && pointer <= range->last_pointer)
            return range;
    }
    return NULL;
}

static const QwCharsetGb18030Range *find_gb18030_codepoint(uint32_t codepoint)
{
    size_t i;
    for (i = 0u; i < QW_CHARSET_GB18030_RANGE_COUNT; ++i) {
        const QwCharsetGb18030Range *range = qw_charset_gb18030_ranges + i;
        uint32_t last_codepoint = range->first_codepoint +
            (range->last_pointer - range->first_pointer);
        if (codepoint >= range->first_codepoint && codepoint <= last_codepoint)
            return range;
    }
    return NULL;
}

static uint32_t gb18030_pointer(const uint8_t *data)
{
    uint32_t value = (uint32_t)(data[0] - 0x81u);
    value = value * 10u + (uint32_t)(data[1] - 0x30u);
    value = value * 126u + (uint32_t)(data[2] - 0x81u);
    value = value * 10u + (uint32_t)(data[3] - 0x30u);
    return value;
}

static void gb18030_bytes(uint32_t pointer, uint8_t *output)
{
    output[3] = (uint8_t)(0x30u + pointer % 10u);
    pointer /= 10u;
    output[2] = (uint8_t)(0x81u + pointer % 126u);
    pointer /= 126u;
    output[1] = (uint8_t)(0x30u + pointer % 10u);
    pointer /= 10u;
    output[0] = (uint8_t)(0x81u + pointer);
}

static QwCharsetStatus decode_legacy(QwCharsetEncoding encoding,
                                    const uint8_t *input, size_t input_size,
                                    size_t *position, uint32_t *codepoint)
{
    const QwCharsetMapEntry *decode;
    const QwCharsetMapEntry *encode;
    size_t decode_count;
    size_t encode_count;
    size_t at = *position;
    size_t width;
    uint32_t key;
    const QwCharsetMapEntry *entry;
    if (!encoding_tables(encoding, &decode, &decode_count,
                         &encode, &encode_count)) {
        return QW_CHARSET_ERR_UNSUPPORTED_ENCODING;
    }
    (void)encode;
    (void)encode_count;
    if (encoding == QW_ENCODING_GB18030 && input_size - at >= 4u &&
        input[at] >= 0x81u && input[at] <= 0xfeu &&
        input[at + 1u] >= 0x30u && input[at + 1u] <= 0x39u &&
        input[at + 2u] >= 0x81u && input[at + 2u] <= 0xfeu &&
        input[at + 3u] >= 0x30u && input[at + 3u] <= 0x39u) {
        uint32_t pointer = gb18030_pointer(input + at);
        const QwCharsetGb18030Range *range = find_gb18030_pointer(pointer);
        if (!range) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        *codepoint = range->first_codepoint + (pointer - range->first_pointer);
        *position = at + 4u;
        return QW_CHARSET_OK;
    }
    if (encoding == QW_ENCODING_EUCJP && input[at] == 0x8fu) {
        if (input_size - at < 3u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        key = ((uint32_t)input[at] << 16) |
              ((uint32_t)input[at + 1u] << 8) | input[at + 2u];
        entry = find_map_entry(decode, decode_count, key);
        if (!entry || entry->width != 3u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        *codepoint = entry->value;
        *position = at + 3u;
        return QW_CHARSET_OK;
    }
    if (at + 1u < input_size) {
        width = 2u;
        key = ((uint32_t)input[at] << 8) | input[at + 1u];
        entry = find_map_entry(decode, decode_count, key);
        if (entry && entry->width == width) {
            *codepoint = entry->value;
            *position = at + width;
            return QW_CHARSET_OK;
        }
    }
    entry = find_map_entry(decode, decode_count, input[at]);
    if (!entry || entry->width != 1u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
    *codepoint = entry->value;
    *position = at + 1u;
    return QW_CHARSET_OK;
}

static QwCharsetStatus decode_next(QwCharsetEncoding encoding,
                                   const uint8_t *input, size_t input_size,
                                   size_t *position, uint32_t *codepoint)
{
    size_t at = *position;
    uint8_t first;
    if (at >= input_size) return QW_CHARSET_ERR_INVALID_SEQUENCE;
    first = input[at];
    switch (encoding) {
    case QW_ENCODING_UTF8: {
        size_t count;
        uint32_t value;
        uint32_t minimum;
        size_t i;
        if (first < 0x80u) {
            *codepoint = first;
            *position = at + 1u;
            return QW_CHARSET_OK;
        }
        if (first >= 0xc2u && first <= 0xdfu) {
            count = 2u; value = first & 0x1fu; minimum = 0x80u;
        } else if (first >= 0xe0u && first <= 0xefu) {
            count = 3u; value = first & 0x0fu; minimum = 0x800u;
        } else if (first >= 0xf0u && first <= 0xf4u) {
            count = 4u; value = first & 0x07u; minimum = 0x10000u;
        } else return QW_CHARSET_ERR_INVALID_SEQUENCE;
        if (count > input_size - at) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        for (i = 1u; i < count; ++i) {
            uint8_t next = input[at + i];
            if ((next & 0xc0u) != 0x80u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
            value = (value << 6) | (uint32_t)(next & 0x3fu);
        }
        if (value < minimum || !valid_codepoint(value))
            return QW_CHARSET_ERR_INVALID_SEQUENCE;
        *codepoint = value;
        *position = at + count;
        return QW_CHARSET_OK;
    }
    case QW_ENCODING_UTF16LE:
    case QW_ENCODING_UTF16BE: {
        int be = encoding == QW_ENCODING_UTF16BE;
        uint16_t first_unit;
        if (input_size - at < 2u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        first_unit = read_u16(input + at, be);
        if (first_unit >= 0xd800u && first_unit <= 0xdbffu) {
            uint16_t second_unit;
            if (input_size - at < 4u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
            second_unit = read_u16(input + at + 2u, be);
            if (second_unit < 0xdc00u || second_unit > 0xdfffu)
                return QW_CHARSET_ERR_INVALID_SEQUENCE;
            *codepoint = 0x10000u +
                (((uint32_t)first_unit - 0xd800u) << 10) +
                ((uint32_t)second_unit - 0xdc00u);
            *position = at + 4u;
            return QW_CHARSET_OK;
        }
        if (first_unit >= 0xdc00u && first_unit <= 0xdfffu)
            return QW_CHARSET_ERR_INVALID_SEQUENCE;
        *codepoint = first_unit;
        *position = at + 2u;
        return QW_CHARSET_OK;
    }
    case QW_ENCODING_UTF32LE:
    case QW_ENCODING_UTF32BE: {
        int be = encoding == QW_ENCODING_UTF32BE;
        uint32_t value;
        if (input_size - at < 4u) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        value = read_u32(input + at, be);
        if (!valid_codepoint(value)) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        *codepoint = value;
        *position = at + 4u;
        return QW_CHARSET_OK;
    }
    case QW_ENCODING_ASCII:
        if (first > 0x7fu) return QW_CHARSET_ERR_INVALID_SEQUENCE;
        *codepoint = first;
        *position = at + 1u;
        return QW_CHARSET_OK;
    case QW_ENCODING_LATIN1:
        *codepoint = first;
        *position = at + 1u;
        return QW_CHARSET_OK;
    case QW_ENCODING_WINDOWS1252:
        *codepoint = first >= 0x80u && first <= 0x9fu
            ? windows_1252_high[first - 0x80u] : first;
        *position = at + 1u;
        return QW_CHARSET_OK;
    default:
        return decode_legacy(encoding, input, input_size, position, codepoint);
    }
}

static QwCharsetStatus encode_legacy(QwCharsetEncoding encoding,
                                    uint32_t codepoint, uint32_t *value,
                                    size_t *width)
{
    const QwCharsetMapEntry *decode;
    const QwCharsetMapEntry *encode;
    size_t decode_count;
    size_t encode_count;
    const QwCharsetMapEntry *entry;
    if (!encoding_tables(encoding, &decode, &decode_count,
                         &encode, &encode_count)) {
        return QW_CHARSET_ERR_UNSUPPORTED_ENCODING;
    }
    (void)decode;
    (void)decode_count;
    entry = find_map_entry(encode, encode_count, codepoint);
    if (entry) {
        *value = entry->value;
        *width = entry->width;
        return QW_CHARSET_OK;
    }
    if (encoding == QW_ENCODING_GB18030) {
        const QwCharsetGb18030Range *range = find_gb18030_codepoint(codepoint);
        if (range) {
            *value = range->first_pointer +
                     (codepoint - range->first_codepoint);
            *width = 4u;
            return QW_CHARSET_OK;
        }
    }
    return QW_CHARSET_ERR_UNREPRESENTABLE;
}

static QwCharsetStatus encoded_width(QwCharsetEncoding encoding,
                                    uint32_t codepoint, size_t *width)
{
    uint32_t value;
    size_t legacy_width;
    QwCharsetStatus status;
    if (!valid_codepoint(codepoint)) return QW_CHARSET_ERR_INVALID_SEQUENCE;
    switch (encoding) {
    case QW_ENCODING_UTF8:
        *width = codepoint <= 0x7fu ? 1u :
                 codepoint <= 0x7ffu ? 2u :
                 codepoint <= 0xffffu ? 3u : 4u;
        return QW_CHARSET_OK;
    case QW_ENCODING_UTF16LE:
    case QW_ENCODING_UTF16BE:
        *width = codepoint <= 0xffffu ? 2u : 4u;
        return QW_CHARSET_OK;
    case QW_ENCODING_UTF32LE:
    case QW_ENCODING_UTF32BE:
        *width = 4u;
        return QW_CHARSET_OK;
    case QW_ENCODING_ASCII:
        if (codepoint > 0x7fu) return QW_CHARSET_ERR_UNREPRESENTABLE;
        *width = 1u;
        return QW_CHARSET_OK;
    case QW_ENCODING_LATIN1:
        if (codepoint > 0xffu) return QW_CHARSET_ERR_UNREPRESENTABLE;
        *width = 1u;
        return QW_CHARSET_OK;
    case QW_ENCODING_WINDOWS1252: {
        size_t i;
        if (codepoint <= 0x7fu || (codepoint >= 0xa0u && codepoint <= 0xffu)) {
            *width = 1u;
            return QW_CHARSET_OK;
        }
        for (i = 0u; i < sizeof(windows_1252_high) / sizeof(windows_1252_high[0]); ++i) {
            if (windows_1252_high[i] == codepoint) {
                *width = 1u;
                return QW_CHARSET_OK;
            }
        }
        return QW_CHARSET_ERR_UNREPRESENTABLE;
    }
    default:
        status = encode_legacy(encoding, codepoint, &value, &legacy_width);
        if (status == QW_CHARSET_OK) *width = legacy_width;
        return status;
    }
}

static void write_u16(uint8_t *output, uint16_t value, int big_endian)
{
    if (big_endian) {
        output[0] = (uint8_t)(value >> 8);
        output[1] = (uint8_t)value;
    } else {
        output[0] = (uint8_t)value;
        output[1] = (uint8_t)(value >> 8);
    }
}

static void write_u32(uint8_t *output, uint32_t value, int big_endian)
{
    if (big_endian) {
        output[0] = (uint8_t)(value >> 24);
        output[1] = (uint8_t)(value >> 16);
        output[2] = (uint8_t)(value >> 8);
        output[3] = (uint8_t)value;
    } else {
        output[0] = (uint8_t)value;
        output[1] = (uint8_t)(value >> 8);
        output[2] = (uint8_t)(value >> 16);
        output[3] = (uint8_t)(value >> 24);
    }
}

static void encode_codepoint(QwCharsetEncoding encoding, uint32_t codepoint,
                             uint8_t *output)
{
    switch (encoding) {
    case QW_ENCODING_UTF8:
        if (codepoint <= 0x7fu) output[0] = (uint8_t)codepoint;
        else if (codepoint <= 0x7ffu) {
            output[0] = (uint8_t)(0xc0u | (codepoint >> 6));
            output[1] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        } else if (codepoint <= 0xffffu) {
            output[0] = (uint8_t)(0xe0u | (codepoint >> 12));
            output[1] = (uint8_t)(0x80u | ((codepoint >> 6) & 0x3fu));
            output[2] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        } else {
            output[0] = (uint8_t)(0xf0u | (codepoint >> 18));
            output[1] = (uint8_t)(0x80u | ((codepoint >> 12) & 0x3fu));
            output[2] = (uint8_t)(0x80u | ((codepoint >> 6) & 0x3fu));
            output[3] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        }
        break;
    case QW_ENCODING_UTF16LE:
    case QW_ENCODING_UTF16BE: {
        int be = encoding == QW_ENCODING_UTF16BE;
        if (codepoint <= 0xffffu) write_u16(output, (uint16_t)codepoint, be);
        else {
            uint32_t adjusted = codepoint - 0x10000u;
            write_u16(output, (uint16_t)(0xd800u + (adjusted >> 10)), be);
            write_u16(output + 2u, (uint16_t)(0xdc00u + (adjusted & 0x3ffu)), be);
        }
        break;
    }
    case QW_ENCODING_UTF32LE:
    case QW_ENCODING_UTF32BE:
        write_u32(output, codepoint, encoding == QW_ENCODING_UTF32BE);
        break;
    case QW_ENCODING_ASCII:
    case QW_ENCODING_LATIN1:
        output[0] = (uint8_t)codepoint;
        break;
    case QW_ENCODING_WINDOWS1252:
        if (codepoint <= 0x7fu || (codepoint >= 0xa0u && codepoint <= 0xffu)) {
            output[0] = (uint8_t)codepoint;
        } else {
            size_t i;
            for (i = 0u; i < sizeof(windows_1252_high) / sizeof(windows_1252_high[0]); ++i) {
                if (windows_1252_high[i] == codepoint) {
                    output[0] = (uint8_t)(0x80u + i);
                    break;
                }
            }
        }
        break;
    default: {
        uint32_t value = 0u;
        size_t width = 0u;
        size_t i;
        (void)encode_legacy(encoding, codepoint, &value, &width);
        if (encoding == QW_ENCODING_GB18030 && width == 4u &&
            value < 126u * 10u * 126u * 10u) {
            gb18030_bytes(value, output);
        } else {
            for (i = width; i > 0u; --i) {
                output[i - 1u] = (uint8_t)value;
                value >>= 8;
            }
        }
        break;
    }
    }
}

static QwCharsetStatus convert_pass(QwCharsetEncoding source_encoding,
                                   QwCharsetEncoding destination_encoding,
                                   const uint8_t *input, size_t input_size,
                                   uint8_t *output, size_t output_capacity,
                                   size_t *output_size)
{
    size_t input_position = 0u;
    size_t written = 0u;
    while (input_position < input_size) {
        uint32_t codepoint;
        size_t width;
        QwCharsetStatus status = decode_next(source_encoding, input, input_size,
                                             &input_position, &codepoint);
        if (status != QW_CHARSET_OK) return status;
        status = encoded_width(destination_encoding, codepoint, &width);
        if (status != QW_CHARSET_OK) return status;
        if (width > SIZE_MAX - written) return QW_CHARSET_ERR_RESOURCE_LIMIT;
        if (output && (written > output_capacity ||
                       width > output_capacity - written))
            return QW_CHARSET_ERR_BUFFER_TOO_SMALL;
        if (output) encode_codepoint(destination_encoding, codepoint, output + written);
        written += width;
    }
    *output_size = written;
    return QW_CHARSET_OK;
}

uint32_t qw_charset_abi_version(void)
{
    return 0x00010000u;
}

QwCharsetStatus qw_charset_convert(const char *source_encoding,
                                   const char *destination_encoding,
                                   const void *input, size_t input_size,
                                   void *output, size_t output_capacity,
                                   size_t *output_size)
{
    QwCharsetEncoding source;
    QwCharsetEncoding destination;
    size_t required = 0u;
    QwCharsetStatus status;
    if (!output_size) return QW_CHARSET_ERR_INVALID_ARGUMENT;
    *output_size = 0u;
    if (!source_encoding || !destination_encoding ||
        (!input && input_size) || (!output && output_capacity)) {
        return QW_CHARSET_ERR_INVALID_ARGUMENT;
    }
    if (!charset_name(source_encoding, &source) ||
        !charset_name(destination_encoding, &destination)) {
        return QW_CHARSET_ERR_UNSUPPORTED_ENCODING;
    }
    status = convert_pass(source, destination, (const uint8_t *)input,
                          input_size, NULL, 0u, &required);
    if (status != QW_CHARSET_OK) return status;
    *output_size = required;
    if (!output) return QW_CHARSET_OK;
    if (output_capacity < required) return QW_CHARSET_ERR_BUFFER_TOO_SMALL;
    status = convert_pass(source, destination, (const uint8_t *)input,
                          input_size, (uint8_t *)output, output_capacity,
                          output_size);
    return status;
}
