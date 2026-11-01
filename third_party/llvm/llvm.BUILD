load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "clang_headers",
    hdrs = glob([
        "include/clang/**",
        "include/clang-c/**",
        "include/llvm/**",
        "include/llvm-c/**",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "clang",
    srcs = ["bin/clang"],
    # NOTE: Clang finds its built-in and libc++ headers relative to itself.
    data = glob([
        "include/c++/v1/**",
        "lib/clang/*/include/**",
    ]),
    visibility = ["//visibility:public"],
)
