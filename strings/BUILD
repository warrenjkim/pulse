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
        "@googletest//:gtest_main",
    ],
)
