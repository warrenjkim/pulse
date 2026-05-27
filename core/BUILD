cc_library(
    name = "core",
    visibility = ["//visibility:public"],
    deps = [
        ":enum_macros",
        ":error",
        ":result",
        ":result_or_die",
        ":stringify",
    ],
)

cc_library(
    name = "enum_macros",
    hdrs = ["enum_macros.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [":stringify"],
)

cc_library(
    name = "stringify",
    hdrs = [
        "container_stringify.h",
        "stringify.h",
    ],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "log",
    hdrs = ["log.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "error",
    hdrs = ["error.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//core:enum_macros",
        "//core:stringify",
        "//strings:cat",
    ],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":error",
        "//core:stringify",
        "//strings:cat",
    ],
)

cc_library(
    name = "result_or_die",
    hdrs = ["result_or_die.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":log",
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
