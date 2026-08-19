load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "output",
    hdrs = ["output.h"],
)

cc_library(
    name = "run",
    srcs = ["run.cc"],
    hdrs = ["run.h"],
    deps = [
        ":output",
        "//pulse/core:error",
        "//pulse/core:result",
    ],
)

cc_test(
    name = "run_test",
    srcs = ["run_test.cc"],
    deps = [
        ":output",
        ":run",
        "//pulse/core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
