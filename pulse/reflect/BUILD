load("@rules_cc//cc:defs.bzl", "cc_test")
load("//bazel:cc.bzl", "cc_library")

cc_library(
    name = "reflect",
    visibility = ["//visibility:public"],
    deps = [
        ":mirror",
        ":schema",
    ],
)

cc_library(
    name = "schema",
    hdrs = ["schema.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "mirror",
    hdrs = ["mirror.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "mirror_test",
    srcs = ["mirror_test.cc"],
    deps = [
        ":mirror",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "stringify",
    hdrs = ["stringify.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":mirror",
        "//pulse/core:pretty_function",
        "//pulse/core:stringify",
    ],
)

cc_library(
    name = "test_types",
    testonly = True,
    hdrs = ["testdata/types.h"],
    deps = [":mirror"],
)

cc_test(
    name = "stringify_test",
    srcs = ["stringify_test.cc"],
    deps = [
        ":stringify",
        ":test_types",
        "//pulse/core:stringify",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
