cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
    include_prefix = "pulse/http/internal",
    strip_include_prefix = ".",
    visibility = ["//http:__pkg__"],
    deps = [
        "//dsa:error",
        "//dsa:result",
        "//http:request",
        "//http:response",
    ],
)

cc_test(
    name = "transform_test",
    srcs = ["transform_test.cc"],
    deps = [
        ":transform",
        "//dsa:result",
        "//http:request",
        "//http:response",
        "@googletest//:gtest_main",
    ],
)
