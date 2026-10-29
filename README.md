# Quirewise Charset

Quirewise Charset is a standalone C11 package for strict conversion between
UTF-8, UTF-16LE/BE, UTF-32LE/BE, ASCII, ISO-8859-1 (Latin-1), and Windows-1252.
It has no third-party or Quirewise package dependencies and exports the
`Quirewise::Charset` CMake target.

`qw_charset_convert` converts length-delimited byte sequences without adding a
terminator. Pass a null output and zero capacity to query the required output
size. The function validates the complete input before writing; malformed input
and unrepresentable characters produce explicit statuses, and insufficient
output capacity leaves the buffer untouched. Input and output ranges must not
overlap.

Named multibyte encodings such as GBK, GB18030, Big5, Shift-JIS, and EUC are not
implemented by this initial package release. Their conversion behavior will be
added as required by the M1-05 Data/Text migration and recorded in the supported
encoding matrix.
