cc_library(
    name = "html",
    visibility = ["//visibility:public"],
    deps = [
        ":attributes",
        ":element",
        ":tags",
        ":text",
    ],
)

cc_library(
    name = "attributes",
    hdrs = ["attributes.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "tags",
    hdrs = ["tags.h"],
    visibility = ["//visibility:public"],
    deps = [":attributes"],
)

cc_library(
    name = "text",
    hdrs = ["text.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "element",
    srcs = ["element.cc"],
    hdrs = ["element.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":attributes",
        ":tags",
        ":text",
        "//pulse/core:overload",
        "//pulse/core:stringify",
        "//pulse/strings:cat",
    ],
)

cc_test(
    name = "element_test",
    srcs = ["element_test.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":attributes",
        ":element",
        ":tags",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
