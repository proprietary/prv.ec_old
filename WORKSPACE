load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

http_archive(
    name = "org_libuv_libuv",
    url = "https://dist.libuv.org/dist/v1.38.1/libuv-v1.38.1.tar.gz",
    sha256 = "0ece7d279e480fa386b066130a562ad1a622079d43d1c30731f2f66cd3f5c647",
    strip_prefix = "libuv-v1.38.1",
    build_file = "//third_party:libuv.BUILD.bazel",
)

http_archive(
    name = "com_github_google_snappy",
    url = "https://github.com/google/snappy/archive/1.1.8.tar.gz",
    sha256 = "16b677f07832a612b0836178db7f374e414f94657c138e6993cbfc5dcc58651f",
    strip_prefix = "snappy-1.1.8",
    build_file = "//third_party:snappy.BUILD.bazel",
)

http_archive(
    name = "com_github_facebook_zstd",
    url = "https://github.com/facebook/zstd/releases/download/v1.4.5/zstd-1.4.5.tar.gz",
    sha256 = "98e91c7c6bf162bf90e4e70fdbc41a8188b9fa8de5ad840c401198014406ce9e",
    strip_prefix = "zstd-1.4.5/lib",
    build_file = "//third_party:zstd.BUILD.bazel",
)

http_archive(
    name = "net_zlib",
    url = "https://zlib.net/zlib-1.2.11.tar.gz",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    build_file = "//third_party:zlib.BUILD.bazel",
)

http_archive(
    name = "com_github_lz4_lz4",
    url = "https://github.com/lz4/lz4/archive/v1.9.2.tar.gz",
    sha256 = "658ba6191fa44c92280d4aa2c271b0f4fbc0e34d249578dd05e50e76d0e5efcc",
    strip_prefix = "lz4-1.9.2",
    build_file = "//third_party:lz4.BUILD.bazel",
)

http_archive(
    name = "org_sourceware_bzip2",
    url = "https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz",
    sha256 = "ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269",
    strip_prefix = "bzip2-1.0.8",
    build_file = "//third_party:bzip2.BUILD.bazel",
)

http_archive(
    name = "com_github_facebook_rocksdb",
    url = "https://github.com/facebook/rocksdb/archive/v6.11.4.tar.gz",
    sha256 = "6793ef000a933af4a834b59b0cd45d3a03a3aac452a68ae669fb916ddd270532",
    strip_prefix = "rocksdb-6.11.4",
    build_file = "//third_party:rocksdb.BUILD.bazel",
)

git_repository(
    name = "com_googlesource_boringssl",
    remote = "https://boringssl.googlesource.com/boringssl",
    commit = "8d5a33f6ec0d781a261ecfc7c18981a64452deae",
    shallow_since = "1598048983 +0000",
)

git_repository(
    name = "com_github_google_googletest",
    remote = "https://github.com/google/googletest",
    commit = "adeef192947fbc0f68fa14a6c494c8df32177508",
    shallow_since = "1597389384 -0400",
)

git_repository(
    name = "com_github_google_flatbuffers",
    remote = "https://github.com/google/flatbuffers",
    commit = "eeacc53d227038ae404562806f9440b13d594d51",
    shallow_since = "1598125149 +0300",
)

new_git_repository(
    name = "com_github_unetworking_usockets",
    remote = "https://github.com/uNetworking/uSockets",
    commit = "c08855070bbd433964a6f0230a91209de6cf6e1f",
    shallow_since = "1598265449 +0200",
    build_file = "//third_party:usockets.BUILD.bazel",
)

new_git_repository(
    name = "com_github_unetworking_uwebsockets",
    remote = "https://github.com/uNetworking/uWebSockets",
    commit = "7420c1a09a7ec34a5b6c6475d24efc5507605d5e",
    shallow_since = "1598265557 +0200",
    build_file = "//third_party:uwebsockets.BUILD.bazel",
)

http_archive(
   name = "se_haxx_curl",
   build_file = "//third_party:curl.BUILD.bazel",
   sha256 = "01ae0c123dee45b01bbaef94c0bc00ed2aec89cb2ee0fd598e0d302a6b5e0a98",
   strip_prefix = "curl-7.69.1",
   url = "https://curl.haxx.se/download/curl-7.69.1.tar.gz",
)

http_archive(
    name = "build_bazel_rules_nodejs",
    sha256 = "10fffa29f687aa4d8eb6dfe8731ab5beb63811ab00981fc84a93899641fd4af1",
    urls = ["https://github.com/bazelbuild/rules_nodejs/releases/download/2.0.3/rules_nodejs-2.0.3.tar.gz"],
)



load("@build_bazel_rules_nodejs//:index.bzl", "node_repositories", "yarn_install")

node_repositories(
    node_repositories = {
        "14.9.0-linux_amd64": ("node-v14.9.0-linux-x64.tar.xz", "node-v14.9.0-linux-x64", "ded70899f43cf8138f88b838aecff5045e763bcab91c4b7f57fe5b69c6722df4"),
        "14.9.0-darwin_amd64": ("node-v14.9.0-darwin-x64.tar.gz", "node-v14.9.0-darwin-x64", "8427e07e3ca70d6ccf5274dde535c9a42b7f873f5a086323eaf2406cdb324daf"),
        "14.9.0-windows_amd64": ("node-v14.9.0-win-x64.zip", "node-v14.9.0-win-x64", "bcd3fc61739e7ac9a4b6103da3fe5f8c9e310b7b0f1b1f0200d5a4b5dd65d723"),
    },
    yarn_repositories = {
        "1.22.5": ("yarn-v1.22.5.tar.gz", "yarn-v1.22.5", "c664fb4692e4dfea750a37a533780834b40198c00cef4bbc5e8c14abab2ac141"),
    },
    yarn_urls = ["https://github.com/yarnpkg/yarn/releases/download/v{version}/{filename}"],
    node_urls = ["https://nodejs.org/dist/v{version}/{filename}"],
    node_version = "14.9.0",
    yarn_version = "1.22.5",
    package_json = ["//web_client:package.json"],
)

# npm_install(
#     name = "npm",
#     package_json = "//web_client:package.json",
#     package_lock_json = "//web_client:package-lock.json",
# )

yarn_install(
    name = "npm",
    package_json = "//web_client:package.json",
    yarn_lock = "//web_client:yarn.lock",
)
