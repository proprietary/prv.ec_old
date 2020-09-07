==============================
Base-58 encoder/decoder in C++
==============================

This library implements base-58 decoding and encoding. Unlike base-64,
base-58 is more readable, URL and filename safe, and doesn't use
characters that are easily confused for one another.

- '0' and 'O' (neither are in base-58 encoded strings)
- 'I' and 'l' (neither are in base-58 encoded strings)

