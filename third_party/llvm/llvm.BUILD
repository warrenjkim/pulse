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
