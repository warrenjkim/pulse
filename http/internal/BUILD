cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
    include_prefix = "pulse/http/internal",
    strip_include_prefix = ".",
    visibility = ["//visibility:__pkg__"],
    deps = [
        "//dsa:error",
        "//dsa:result",
        "//http:types",
    ],
)

cc_test(
    name = "transform_test",
    srcs = ["transform_test.cc"],
    deps = [
        ":transform",
        "//dsa:result",
        "//http:types",
        "@googletest//:gtest_main",
    ],
)
