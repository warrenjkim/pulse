cc_library(
    name = "strings",
    visibility = ["//visibility:public"],
    deps = [
        ":cat",
        ":split",
    ],
)

cc_library(
    name = "cat",
    hdrs = ["cat.h"],
    include_prefix = "pulse/strings",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "split",
    srcs = ["split.cc"],
    hdrs = ["split.h"],
    include_prefix = "pulse/strings",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "split_test",
    srcs = ["split_test.cc"],
    deps = [
        ":split",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
