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

