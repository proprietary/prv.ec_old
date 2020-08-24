=================
Development Guide
=================

Building
--------

Dependencies
~~~~~~~~~~~~

- C++20 compiler toolchain such as gcc-10 or clang-11
- Bazel
- Linux

This project was designed with GNU/Linux in mind. But the code uses
standard C++ and C, so it's possible to get it to work anywhere. You
should be able to build on alternative platforms like macOS, Windows,
iOS, Android, OpenBSD, or FreeBSD but this will take work and is not
yet supported.

Build system
~~~~~~~~~~~~

Easiest way to get started with Bazel is Bazelisk. Install Bazelisk
https://github.com/bazelbuild/bazelisk

Then in your shell or in .zshrc/.zshenv/.bashrc/etc.::

  alias bazel=bazelisk

Building the entire project is as easy as::

  bazel build //:all

Using clang tooling
-------------------

clang-format, clang-tidy, clangd are all invaluable tools. They
require a compilation database i.e. a "compile_commands.json" at the
root of the source tree.

Generate compile_commands.json for Bazel with https://github.com/grailbio/bazel-compilation-database

