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
    QW_ENCODING_WINDOWS1252
} QwCharsetEncoding;

static const uint16_t windows_1252_high[32] = {
    0x20acu, 0x0081u, 0x201au, 0x0192u, 0x201eu, 0x2026u, 0x2020u, 0x2021u,
    0x02c6u, 0x2030u, 0x0160u, 0x2039u, 0x0152u, 0x008du, 0x017du, 0x008fu,
    0x0090u, 0x2018u, 0x2019u, 0x201cu, 0x201du, 0x2022u, 0x2013u, 0x2014u,
    0x02dcu, 0x2122u, 0x0161u, 0x203au, 0x0153u, 0x009du, 0x017eu, 0x0178u
};

static int charset_name(const char *name, QwCharsetEncoding *encoding)
{
    char normalized[40];
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
    }
    return QW_CHARSET_ERR_UNSUPPORTED_ENCODING;
}

static QwCharsetStatus encoded_width(QwCharsetEncoding encoding,
                                    uint32_t codepoint, size_t *width)
{
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
    }
    return QW_CHARSET_ERR_UNSUPPORTED_ENCODING;
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
