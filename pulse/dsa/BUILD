cc_library(
    name = "dsa",
    visibility = ["//visibility:public"],
    deps = [
        ":trie",
    ],
)

cc_library(
    name = "trie",
    hdrs = ["trie.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "trie_test",
    srcs = ["trie_test.cc"],
    deps = [
        ":trie",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
