# Quirewise Charset

Quirewise Charset is a standalone C11 package for strict conversion between
Unicode encodings and the legacy character sets used by Quirewise Data and Text.
It has no third-party or Quirewise package dependencies and exports the
`Quirewise::Charset` CMake target.

Supported encodings are UTF-8, UTF-16LE/BE, UTF-32LE/BE, ASCII, ISO-8859-1,
Windows-1250 through Windows-1258, ISO-8859-7, KOI8-R/U, DOS code pages
437/850/852/866/874, Mac Roman, GBK, GB2312, GB18030, Big5, CP950, CP932
(Shift-JIS), CP949, EUC-JP, and EUC-KR. EUC-TW is not supported. Legacy mapping
tables are committed as generated C data; Python is used only by the optional
table generation script and is not required to build or use the package.

`qw_charset_convert` converts length-delimited byte sequences without adding a
terminator. Pass a null output and zero capacity to query the required output
size. The function validates the complete input before writing; malformed input
and unrepresentable characters produce explicit statuses, and insufficient
output capacity leaves the buffer untouched. Input and output ranges must not
overlap.
