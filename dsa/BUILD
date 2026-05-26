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
    deps = [
        "//core:enum_macros",
        "//core:stringify",
    ],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    include_prefix = "pulse/dsa",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":error",
        "//core:stringify",
    ],
)

cc_library(
    name = "result_or_die",
    hdrs = ["result_or_die.h"],
    include_prefix = "pulse/dsa",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":result",
    ],
)

cc_test(
    name = "result_or_die_test",
    srcs = ["result_or_die_test.cc"],
    deps = [
        ":error",
        ":result",
        ":result_or_die",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
