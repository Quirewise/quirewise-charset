"""Generate immutable conversion maps from the Python standard codecs.

The generated C tables contain character mapping data only. The package does
not link to Python and does not use Python at build or runtime.

Mapping output can vary with the Python codec data shipped by a release. The
generated header records the Python version used to produce the committed data.
"""

from pathlib import Path
import sys


def decode_scalar(raw, codec):
    try:
        text = raw.decode(codec, "strict")
    except UnicodeDecodeError:
        return None
    if len(text) != 1:
        return None
    value = ord(text)
    return value if value <= 0x10FFFF and not 0xD800 <= value <= 0xDFFF else None


def encoded_value(raw):
    return int.from_bytes(raw, "big")


def iter_sequences(name):
    for value in range(256):
        yield bytes((value,))
    if name in ("gbk", "gb18030"):
        for lead in range(0x81, 0xFF):
            for trail in (*range(0x40, 0x7F), *range(0x80, 0xFF)):
                yield bytes((lead, trail))
    elif name == "gb2312":
        for lead in range(0xA1, 0xF8):
            for trail in range(0xA1, 0xFF):
                yield bytes((lead, trail))
    elif name in ("big5", "cp950"):
        for lead in range(0x81, 0xFF):
            for trail in (*range(0x40, 0x7F), *range(0xA1, 0xFF)):
                yield bytes((lead, trail))
    elif name == "cp932":
        leads = (*range(0x81, 0xA0), *range(0xE0, 0xFD))
        trails = (*range(0x40, 0x7F), *range(0x80, 0xFD))
        for lead in leads:
            for trail in trails:
                yield bytes((lead, trail))
    elif name == "euc_jp":
        for lead in range(0xA1, 0xFF):
            for trail in range(0xA1, 0xFF):
                yield bytes((lead, trail))
        for trail in range(0xA1, 0xE0):
            yield bytes((0x8E, trail))
        for middle in range(0xA1, 0xFF):
            for trail in range(0xA1, 0xFF):
                yield bytes((0x8F, middle, trail))
    elif name == "euc_kr":
        for lead in range(0xA1, 0xFF):
            for trail in range(0xA1, 0xFF):
                yield bytes((lead, trail))
    elif name == "cp949":
        for lead in range(0x81, 0xFF):
            for trail in (*range(0x41, 0x5B), *range(0x61, 0x7B),
                          *range(0x81, 0xFF)):
                yield bytes((lead, trail))


SPECS = [
    ("cp1250", "cp1250"),
    ("cp1251", "cp1251"),
    ("cp1253", "cp1253"),
    ("cp1254", "cp1254"),
    ("cp1255", "cp1255"),
    ("cp1256", "cp1256"),
    ("cp1257", "cp1257"),
    ("cp1258", "cp1258"),
    ("iso8859_7", "iso8859_7"),
    ("koi8_r", "koi8_r"),
    ("koi8_u", "koi8_u"),
    ("cp437", "cp437"),
    ("cp850", "cp850"),
    ("cp852", "cp852"),
    ("cp866", "cp866"),
    ("cp874", "cp874"),
    ("mac_roman", "mac_roman"),
    ("gbk", "gbk"),
    ("gb2312", "gb2312"),
    ("big5", "big5"),
    ("cp950", "cp950"),
    ("cp932", "cp932"),
    ("cp949", "cp949"),
    ("euc_jp", "euc_jp"),
    ("euc_kr", "euc_kr"),
    ("gb18030", "gb18030"),
]


def map_entries(name, codec):
    decoded = {}
    codepoints = set()
    for raw in iter_sequences(name):
        scalar = decode_scalar(raw, codec)
        if scalar is None:
            continue
        code = encoded_value(raw)
        decoded[code] = (scalar, len(raw))
        codepoints.add(scalar)

    encoded = {}
    for scalar in codepoints:
        try:
            raw = chr(scalar).encode(codec, "strict")
        except UnicodeEncodeError:
            continue
        if decode_scalar(raw, codec) == scalar:
            encoded[scalar] = (encoded_value(raw), len(raw))

    decode_rows = [(code, scalar, width)
                   for code, (scalar, width) in sorted(decoded.items())]
    encode_rows = [(scalar, code, width)
                   for scalar, (code, width) in sorted(encoded.items())]
    return decode_rows, encode_rows


def gb18030_ranges():
    max_pointer = 126 * 10 * 126 * 10
    ranges = []
    start = previous_pointer = previous_scalar = None

    def append_current():
        if start is not None:
            ranges.append((start, previous_pointer, previous_scalar - (previous_pointer - start)))

    for pointer in range(max_pointer):
        value = pointer
        b4 = 0x30 + value % 10
        value //= 10
        b3 = 0x81 + value % 126
        value //= 126
        b2 = 0x30 + value % 10
        value //= 10
        b1 = 0x81 + value
        scalar = decode_scalar(bytes((b1, b2, b3, b4)), "gb18030")
        if scalar is None:
            append_current()
            start = previous_pointer = previous_scalar = None
        elif start is None:
            start = previous_pointer = pointer
            previous_scalar = scalar
        elif scalar == previous_scalar + 1:
            previous_pointer = pointer
            previous_scalar = scalar
        else:
            append_current()
            start = previous_pointer = pointer
            previous_scalar = scalar
    append_current()
    return ranges


def render_array(lines, symbol, rows):
    lines.append(f"static const QwCharsetMapEntry {symbol}[] = {{")
    for key, value, width in rows:
        lines.append(f"    {{0x{key:08x}u, 0x{value:08x}u, {width}u}},")
    lines.append("};")
    lines.append(f"#define {symbol.upper()}_COUNT \\")
    lines.append(f"    (sizeof({symbol}) / sizeof({symbol}[0]))")
    lines.append("")


def main():
    lines = [
        "/* Generated by tools/generate_charset_tables.py; do not edit manually. */",
        "/* Mapping data comes from the Python standard codecs identified by the generator. */",
        f"/* Python codec data version: {sys.version.split()[0]} */",
        "",
    ]
    for name, codec in SPECS:
        decode_rows, encode_rows = map_entries(name, codec)
        render_array(lines, f"qw_charset_{name}_decode", decode_rows)
        render_array(lines, f"qw_charset_{name}_encode", encode_rows)
        print(f"{name}: decode={len(decode_rows)}, encode={len(encode_rows)}")

    ranges = gb18030_ranges()
    lines.append("static const QwCharsetGb18030Range qw_charset_gb18030_ranges[] = {")
    for pointer, end, scalar in ranges:
        lines.append(f"    {{0x{pointer:08x}u, 0x{end:08x}u, 0x{scalar:08x}u}},")
    lines.extend([
        "};",
        "#define QW_CHARSET_GB18030_RANGE_COUNT \\",
        "    (sizeof(qw_charset_gb18030_ranges) / sizeof(qw_charset_gb18030_ranges[0]))",
        "",
    ])
    output = Path(__file__).resolve().parents[1] / "src" / "charset_tables.h"
    output.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"GB18030 four-byte ranges: {len(ranges)}")
    print(f"Wrote {output}")


if __name__ == "__main__":
    main()
