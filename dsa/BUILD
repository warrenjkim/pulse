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
        ":trie",
    ],
)

cc_library(
    name = "trie",
    hdrs = [
        "trie.h",
    ],
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
