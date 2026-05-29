cc_library(
    name = "value",
    hdrs = [
        "value.h",
    ],
    include_prefix = "pulse/json",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//json/utils:exception",
    ],
)

cc_test(
    name = "value_test",
    srcs = ["value_test.cc"],
    deps = [
        "//json/utils:exception",
        "//json/value",
        "@googletest//:gtest_main",
    ],
)
