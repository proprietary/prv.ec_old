"""
Rules for embedding files as string literals in C++ binaries
"""

load("@rules_cc//cc:defs.bzl", "cc_library")

strliteral_binary_path = "//third_party/strliteral:strliteral"

def embed_file(
        name,
        srcs,
        outs,
        identifier,
        visibility,
        compatible_with = None,
        restricted_to = None,
        output_to_bindir = False):
    if len(srcs) > 1:
        return
    outfile = "%s_generated.h" % srcs[0].split('/')[-1]
    cmd = " ".join([
        "$(location %s)" % strliteral_binary_path,
        "-i %s" % identifier,
        "$<",
        "$@",
    ])
    native.genrule(
        name = "%s_srcs" % name,
        srcs = srcs,
        outs = outs,
        output_to_bindir = output_to_bindir,
        tools = [strliteral_binary_path],
        cmd = cmd,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
        message = "Generating string literal header file to embed file: %s" % name,
    )
    # native.filegroup(
    #     name = "%s_out" % name,
    #     srcs = [outfile],
    #     visibility = visibility,
    #     compatible_with = compatible_with,
    #     restricted_to = restricted_to,
    # )
    cc_library(
        name = "%s" % name,
        srcs = [":%s_srcs" % name],
        hdrs = [":%s_srcs" % name],
        linkstatic = 1,
        visibility = visibility,
    )
