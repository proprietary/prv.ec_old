==============================================
Base-58 encoder/decoder in little endian (C++)
==============================================

This library implements base-58 decoding and encoding. Unlike base-64,
base-58 is more readable, URL and filename safe, and doesn't use
characters that are easily confused for one another ('0' and 'O', 'I'
and 'l').

Most base-58 encoding and decoding libraries assume big endian byte
ordering. This works for little endian only.
