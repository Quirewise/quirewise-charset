# quirewise-charset

Strict conversion between Unicode and the legacy text encodings used by
Quirewise Data and Text.

`Quirewise::Charset` (CMake package `QuirewiseCharset`) is an independent,
MIT-licensed C11 library with no third-party and no Quirewise package
dependencies. Its whole public API is declared in `quirewise/charset.h`.

## What it does

- Converts length-delimited byte sequences between UTF-8, UTF-16LE/BE,
  UTF-32LE/BE, ASCII, ISO-8859-1, Windows-1250 through Windows-1258,
  ISO-8859-7, KOI8-R/U, CP437/850/852/866/874, Mac Roman, GBK, GB2312,
  GB18030, Big5, CP950, CP932 (Shift-JIS), CP949, EUC-JP and EUC-KR.
- Validates the complete input before writing any output byte. Malformed
  sequences return `QW_CHARSET_ERR_INVALID_SEQUENCE`; characters the
  destination cannot represent return `QW_CHARSET_ERR_UNREPRESENTABLE`. Nothing
  is ever silently replaced.
- Reports the exact output size without writing, so callers can size a buffer
  first or query into a fixed-size buffer.
- Accepts encoding names case-insensitively and ignores `-`, `_`, spaces and
  `.` in them.

It deliberately does **not** detect encodings, sniff BOMs, strip or add a BOM,
normalize Unicode, transliterate, or stream. EUC-TW is not supported. All
mapping tables are committed as generated C data; Python is needed only to
regenerate them, never to build or use the package.

## Requirements

- C11 compiler and CMake 3.21 or newer.
- No third-party libraries and no other Quirewise packages.
- CTest for the test suite.

## Build, test and install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix /path/to/prefix
```

Shared builds are available with `-DQUIREWISE_BUILD_SHARED=ON`.

## Use it from CMake

```cmake
find_package(QuirewiseCharset CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE Quirewise::Charset)
```

The package has no dependencies, so no other prefix is required.

## Quick start

Convert Windows-1252 bytes to UTF-8. The first call asks for the output size;
the second writes into a buffer that is known to be large enough.

```c
#include <quirewise/charset.h>

#include <stdio.h>

int main(void)
{
    static const char input[] = "Caf\xe9"; /* Windows-1252, 4 bytes */
    char output[16];
    size_t output_size = 0u;
    QwCharsetStatus status;

    status = qw_charset_convert("Windows-1252", "UTF-8", input,
                                sizeof(input) - 1u, NULL, 0u, &output_size);
    if (status != QW_CHARSET_OK || output_size > sizeof(output)) {
        fprintf(stderr, "size query failed: %d\n", (int)status);
        return 1;
    }
    status = qw_charset_convert("Windows-1252", "UTF-8", input,
                                sizeof(input) - 1u, output, sizeof(output),
                                &output_size);
    if (status != QW_CHARSET_OK) {
        fprintf(stderr, "conversion failed: %d\n", (int)status);
        return 1;
    }
    fwrite(output, 1u, output_size, stdout);
    fputc('\n', stdout);
    return 0;
}
```

The result is not NUL-terminated: `output_size` is the number of bytes written.

## API overview

| Header | Purpose |
|---|---|
| `quirewise/charset.h` | ABI version and the conversion entry point |

| Task | Functions |
|---|---|
| Version | `qw_charset_abi_version` (returns `0x00010000`) |
| Convert or measure | `qw_charset_convert` |

`qw_charset_convert` takes the source and destination names, input pointer and
size, optional output pointer and capacity, and an `output_size` out-parameter.

## Behavior and limits

- Returns `QW_CHARSET_OK` on success; all errors are positive `QwCharsetStatus`
  values, unlike the shared `QwStatus` enum.
- On success, `*output_size` is the produced byte count. On a sizing query it is
  the required byte count. On `QW_CHARSET_ERR_BUFFER_TOO_SMALL` it is also the
  required size, and the output buffer is untouched.
- On `QW_CHARSET_ERR_INVALID_SEQUENCE` and
  `QW_CHARSET_ERR_UNREPRESENTABLE`, `*output_size` is zero and no output bytes
  are written.
- Input and output must not overlap. The function allocates nothing and retains
  no pointers.
- `input` may be `NULL` only when `input_size` is zero. `output` may be `NULL`
  only when `output_capacity` is zero.
- The output is not terminated and no BOM is added. BOM bytes in the input are
  ordinary characters: they decode to U+FEFF and are re-encoded.
- There are no configured size caps. `QW_CHARSET_ERR_RESOURCE_LIMIT` is reserved
  for a `size_t` overflow while summing the required output size.
- Encoding names are limited to ASCII letters and digits after separators are
  removed, and to 47 normalized characters. Unrecognized names return
  `QW_CHARSET_ERR_UNSUPPORTED_ENCODING`.

## Testing

`ctest` runs `charset_vectors`, covering name aliases, Windows-1252, UTF-16
surrogate pairs, UTF-32 range rejection, GBK, GB18030, Shift-JIS, EUC-JP,
CP949, Windows-1251, Mac Roman, malformed sequences, unrepresentable characters,
buffer-too-small behavior, and zero-length sizing queries.

## Documentation

See [docs/usage.md](docs/usage.md) for the full encoding list, alias rules,
sizing patterns, validation recipes and status handling.

## License

MIT. See [LICENSE](LICENSE).
