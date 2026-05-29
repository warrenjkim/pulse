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
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:stringify",
    ],
)

cc_library(
    name = "split",
    srcs = ["split.cc"],
    hdrs = ["split.h"],
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
