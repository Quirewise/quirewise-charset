# quirewise-charset usage guide

This guide covers the task-level workflows of `Quirewise::Charset`. The public
API is declared in `quirewise/charset.h`.

## 1. Build and integrate

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix /path/to/prefix
```

Consumer CMake:

```cmake
find_package(QuirewiseCharset CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE Quirewise::Charset)
```

There is nothing to initialize at runtime and no dependency on any other
Quirewise package.

## 2. Core model

`qw_charset_convert` works on a length-delimited byte range, not on a C string:

```c
QwCharsetStatus qw_charset_convert(const char *source_encoding,
                                   const char *destination_encoding,
                                   const void *input, size_t input_size,
                                   void *output, size_t output_capacity,
                                   size_t *output_size);
```

Important properties:

- No terminator is required on input and none is added to output.
- The function is stateless and allocates nothing.
- It validates the entire input before writing the first output byte, so a
  failed call leaves `output` untouched.
- `output = NULL` with `output_capacity = 0` is the sizing query.
- A BOM is ordinary content: it decodes to U+FEFF and is re-encoded. The
  function does not add, strip or interpret a BOM.
- There is no encoding detection. The caller must know the source name; this
  package answers "is this byte sequence valid for that name and can it be
  represented in the destination?".

Result conventions:

| Call shape | Meaning |
|---|---|
| `output = NULL`, capacity 0 | measure only; `*output_size` = required bytes |
| `output != NULL`, capacity >= required | convert; `*output_size` = written bytes |
| `output != NULL`, capacity < required | `QW_CHARSET_ERR_BUFFER_TOO_SMALL`; `*output_size` = required bytes |
| Empty input (`size` 0) | `QW_CHARSET_OK`, `*output_size` = 0 |

## 3. Supported encodings and names

Name matching is ASCII case-insensitive. `-`, `_`, space and `.` are ignored
anywhere in the name, so `UTF-8`, `utf_8` and `U T F 8` all match. Any other
non-alphanumeric character, or a normalized name longer than 47 characters, is
rejected as unsupported.

| Encoding | Accepted names (after normalization) |
|---|---|
| UTF-8 | `UTF-8`, `utf8` |
| UTF-16LE / UTF-16BE | `UTF-16LE`, `UTF-16BE` |
| UTF-32LE / UTF-32BE | `UTF-32LE`, `UTF-32BE` |
| ASCII | `ASCII`, `US-ASCII`, `ANSI_X3.4-1968` |
| ISO-8859-1 | `ISO-8859-1`, `Latin-1`, `Latin1`, `Latin`, `L1`, `CP819` |
| Windows-1252 | `Windows-1252`, `CP1252`, `x-cp1252` |
| Windows-1250 | `Windows-1250`, `CP1250`, `x-cp1250` |
| Windows-1251 | `Windows-1251`, `CP1251`, `x-cp1251` |
| Windows-1253 … 1258 | `Windows-125N`, `CP125N` |
| ISO-8859-7 | `ISO-8859-7`, `Greek`, `Greek8` |
| KOI8-R | `KOI8-R`, `KOI`, `KOI8`, `KOI8-RU` |
| KOI8-U | `KOI8-U` |
| CP437 | `CP437`, `IBM437` |
| CP850 | `CP850`, `IBM850` |
| CP852 | `CP852`, `IBM852` |
| CP866 | `CP866`, `IBM866` |
| CP874 | `CP874`, `Windows-874` |
| Mac Roman | `Mac Roman`, `Macintosh`, `x-mac-roman` |
| GBK | `GBK`, `CP936`, `Windows-936`, `x-gbk` |
| GB2312 | `GB2312`, `EUC-CN`, `Chinese`, `CSGB2312`, `CSISO58GB231280`, `ISO-IR-58` |
| GB18030 | `GB18030`, `GB18030-2000` |
| Big5 | `Big5`, `CN-Big5`, `CSBig5`, `x-x-big5`, `Big5-HKSCS` |
| CP950 | `CP950`, `Windows-950` |
| CP932 (Shift-JIS) | `CP932`, `Shift-JIS`, `SJIS`, `MS-Kanji`, `MS932`, `CSShiftJIS`, `Windows-31J`, `x-sjis` |
| CP949 | `CP949`, `Windows-949` |
| EUC-JP | `EUC-JP`, `x-euc-jp`, `CSEUCPKDFMTJAPANESE` |
| EUC-KR | `EUC-KR`, `CSEUCKR`, `CSKSC56011987`, `ISO-IR-149`, `Korean`, `KSC5601` |

Not supported: EUC-TW, UTF-7, ISO-8859-2 through 6 and 8 through 16, Windows
Cyrillic variants beyond CP1251/KOI8, and any multi-byte set not listed above.

## 4. Recipe: size, then convert

```c
size_t needed = 0u;
QwCharsetStatus status;
char *out;

status = qw_charset_convert("Shift-JIS", "UTF-8", input, input_size,
                            NULL, 0u, &needed);
if (status != QW_CHARSET_OK) {
    fprintf(stderr, "invalid Shift-JIS: %d\n", (int)status);
    return 1;
}
out = (char *)malloc(needed ? needed : 1u);
if (!out) return 1;
status = qw_charset_convert("Shift-JIS", "UTF-8", input, input_size,
                            out, needed, &needed);
if (status != QW_CHARSET_OK) {
    free(out);
    return 1;
}
/* out is not NUL-terminated; needed is the exact length. */
fwrite(out, 1u, needed, stdout);
free(out);
```

If you need a C string, allocate `needed + 1` and set `out[needed] = '\0'`
yourself.

## 5. Recipe: fixed buffer with a bounded retry

```c
char stack[512];
size_t needed = 0u;
QwCharsetStatus status = qw_charset_convert(
    "Windows-1251", "UTF-8", input, input_size, NULL, 0u, &needed);

if (status != QW_CHARSET_OK) return 1;
if (needed <= sizeof(stack)) {
    status = qw_charset_convert("Windows-1251", "UTF-8", input, input_size,
                                stack, sizeof(stack), &needed);
} else {
    char *heap = (char *)malloc(needed);
    if (!heap) return 1;
    status = qw_charset_convert("Windows-1251", "UTF-8", input, input_size,
                                heap, needed, &needed);
    /* use heap, then free it */
    free(heap);
}
return status == QW_CHARSET_OK ? 0 : 1;
```

`QW_CHARSET_ERR_BUFFER_TOO_SMALL` is also usable as a retry signal: the call
writes nothing and sets `*output_size` to the required size. Measuring first and
retrying after `BUFFER_TOO_SMALL` perform the same validation; the failed attempt
only adds the capacity check.

## 6. Recipe: validate a declared encoding

Metadata often declares an encoding you cannot trust. Validation without
producing output is a measurement to UTF-8:

```c
size_t ignored = 0u;
if (qw_charset_convert(declared_name, "UTF-8", data, size,
                       NULL, 0u, &ignored) != QW_CHARSET_OK) {
    fprintf(stderr, "payload is not valid %s\n", declared_name);
    return 1;
}
```

This validates the source bytes; conversion to UTF-8 cannot fail with
"unrepresentable". It is not detection: the name still comes from the caller.
If the name is unknown, the call returns
`QW_CHARSET_ERR_UNSUPPORTED_ENCODING`.

## 7. Recipe: transcode between two legacy sets

Converting directly between legacy sets avoids an intermediate application
buffer. Characters missing from the destination fail instead of becoming `?`.

```c
size_t needed = 0u;
QwCharsetStatus status = qw_charset_convert(
    "Windows-1251", "KOI8-R", input, input_size, NULL, 0u, &needed);

if (status == QW_CHARSET_ERR_UNREPRESENTABLE) {
    fprintf(stderr, "KOI8-R cannot represent the whole input\n");
    return 1;
}
if (status != QW_CHARSET_OK) return 1;
/* allocate needed bytes and call again with output */
```

The same behavior applies to ASCII and ISO-8859-1 destinations, which reject
any code point outside their repertoire.

## 8. Error handling

| Status | Value | Meaning |
|---|---|---|
| `QW_CHARSET_OK` | 0 | success |
| `QW_CHARSET_ERR_INVALID_ARGUMENT` | 1 | `output_size` is `NULL`, a name is `NULL`, `input` is `NULL` with nonzero size, or `output` is `NULL` with nonzero capacity |
| `QW_CHARSET_ERR_UNSUPPORTED_ENCODING` | 2 | source or destination name is unknown |
| `QW_CHARSET_ERR_INVALID_SEQUENCE` | 3 | source bytes are truncated, overlong, out of range, or an unpaired surrogate; ASCII bytes above 0x7F also land here |
| `QW_CHARSET_ERR_UNREPRESENTABLE` | 4 | the destination cannot encode a decoded code point |
| `QW_CHARSET_ERR_BUFFER_TOO_SMALL` | 5 | capacity is below the required size; nothing was written |
| `QW_CHARSET_ERR_RESOURCE_LIMIT` | 6 | the required size would overflow `size_t` |

Guidance:

- Check the status, not just `output_size`; a failed call can still report a
  required size for `BUFFER_TOO_SMALL`.
- Do not treat `output` as valid on failure. This library writes nothing on any
  failure path, but your own retry logic should still reset its length variable.
- Treat `UNREPRESENTABLE` as a product decision: reject, substitute at the
  application layer, or pick a wider destination such as UTF-8.

## 9. Memory, lifetime and threading

- The library allocates nothing, keeps no pointers and has no global mutable
  state. All mapping tables are `static const`.
- Input and output ranges must not overlap; there is no internal staging buffer.
- Independent calls are safe on separate threads. The same input buffer may be
  read by several threads concurrently.
- Output size is exact. Worst-case expansion is 4 bytes per code point when the
  destination is UTF-32, 3 bytes for UTF-8 from a single-byte legacy encoding,
  and up to 4 bytes for UTF-16 when a surrogate pair is needed; size the buffer
  from the returned count rather than guessing.

## 10. Limits and notes

- No configured input or output cap. The only limit status is a `size_t`
  overflow while accumulating the required size.
- Legacy tables follow the published code-page mappings. Some double-byte sets
  contain duplicate mappings, so a decode/encode round trip is not guaranteed to
  reproduce the original byte for every duplicate.
- ISO-8859-1 maps all 256 bytes directly. Windows-1252 differs in 0x80–0x9F,
  mapping mostly printable characters such as U+20AC and U+2019, plus a few
  control code points.
- `QW_CHARSET_ERR_UNSUPPORTED_ENCODING` covers EUC-TW and any name not in the
  table above.
- The package does not normalize Unicode, case-fold, transliterate or strip
  control characters.

## 11. Related packages

- `Quirewise::Data` — structured data and text values that may declare a legacy
  charset.
- `Quirewise::Text` — plain-text and HTML readers that consume UTF-8.
- `Quirewise::Ebook` — PML and HTMLZ payloads that may retain a legacy encoding.
- `Quirewise::Pipeline` — whole-file conversion front end.

## 12. License

MIT. See `LICENSE`.
