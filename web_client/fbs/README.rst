=====================
generated flatbuffers
=====================

Run this command to manually re-generate flatbuffers for Typescript in this directory::

  bazel run @com_github_google_flatbuffers//:flatc -- \
    --gen-object-api \
    --reflect-names \
    --no-fb-import \
    --ts \
    -o $(pwd) \
    $(pwd)/../../idl/*.fbs

There is a BUILD file to generate the generated code at build-time
(currently in `web_client/fbs_bzl`) but I still can't figure out a
good way to have it seen by webpack. It seems like it's more trouble
than it's worth, since that invisible generated code wouldn't be seen
by tooling either. You have to remember to run this command in this
directory every time you change the flatbuffer IDL files, which
shouldn't be often. This is a minor inconvenience.
