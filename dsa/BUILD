test_suite(
    name = "tests",
    tests = [
        ":trie_test",
    ],
)

cc_library(
    name = "dsa",
    visibility = ["//visibility:public"],
    deps = [
        ":result",
        ":trie",
    ],
)

cc_library(
    name = "trie",
    hdrs = ["trie.h"],
    include_prefix = "pulse/dsa",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "trie_test",
    srcs = ["trie_test.cc"],
    deps = [
        ":trie",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "error",
    hdrs = ["error.h"],
    include_prefix = "pulse/dsa",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [":error_code"],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    include_prefix = "pulse/dsa",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [":error"],
)
