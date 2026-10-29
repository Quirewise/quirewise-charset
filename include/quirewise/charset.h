#ifndef QUIREWISE_CHARSET_H
#define QUIREWISE_CHARSET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum QwCharsetStatus {
    QW_CHARSET_OK = 0,
    QW_CHARSET_ERR_INVALID_ARGUMENT = 1,
    QW_CHARSET_ERR_UNSUPPORTED_ENCODING = 2,
    QW_CHARSET_ERR_INVALID_SEQUENCE = 3,
    QW_CHARSET_ERR_UNREPRESENTABLE = 4,
    QW_CHARSET_ERR_BUFFER_TOO_SMALL = 5,
    QW_CHARSET_ERR_RESOURCE_LIMIT = 6
} QwCharsetStatus;

uint32_t qw_charset_abi_version(void);

/* Convert a byte sequence between supported encodings without a terminator.
 * Names are ASCII case-insensitive; hyphens, underscores, spaces, and dots
 * are ignored. The initial release supports UTF-8, UTF-16LE/BE, UTF-32LE/BE,
 * ASCII, ISO-8859-1 (Latin-1), and Windows-1252.
 *
 * Set output to NULL and output_capacity to zero to query the required output
 * size. Otherwise output_size receives the required or produced byte count.
 * On QW_CHARSET_ERR_BUFFER_TOO_SMALL no output bytes are written. Input and
 * output buffers must not overlap. The result is not null-terminated. */
QwCharsetStatus qw_charset_convert(const char *source_encoding,
                                   const char *destination_encoding,
                                   const void *input, size_t input_size,
                                   void *output, size_t output_capacity,
                                   size_t *output_size);

#ifdef __cplusplus
}
#endif

#endif
