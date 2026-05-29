cc_library(
    name = "http",
    visibility = ["//visibility:public"],
    deps = [
        ":handler",
        ":method",
        ":pattern",
        ":request",
        ":response",
        ":server",
    ],
)

cc_library(
    name = "method",
    hdrs = ["method.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//core:enum_macros",
        "//core:stringify",
    ],
)

cc_library(
    name = "pattern",
    srcs = ["pattern.cc"],
    hdrs = ["pattern.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//core:enum_macros",
        "//core:error",
        "//core:result",
        "//core:stringify",
        "//strings:cat",
        "//strings:split",
    ],
)

cc_test(
    name = "pattern_test",
    srcs = ["pattern_test.cc"],
    deps = [
        ":pattern",
        "//core:error",
        "//core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "request",
    hdrs = ["request.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":method",
        "//core:stringify",
        "//strings:cat",
    ],
)

cc_library(
    name = "response",
    hdrs = ["response.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//core:stringify",
        "//strings:cat",
    ],
)

cc_library(
    name = "router",
    srcs = ["router.cc"],
    hdrs = ["router.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":handler",
        ":method",
        ":pattern",
        "//core:enum_macros",
        "//core:error",
        "//core:result",
        "//core:type_map",
        "//strings:cat",
        "//strings:split",
    ],
)

cc_test(
    name = "router_test",
    srcs = ["router_test.cc"],
    deps = [
        ":handler",
        ":method",
        ":pattern",
        ":request",
        ":response",
        ":router",
        "//core:error",
        "//core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "handler",
    hdrs = ["handler.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":request",
        ":response",
        "//core:type_map",
    ],
)

cc_library(
    name = "server",
    srcs = ["server.cc"],
    hdrs = ["server.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":handler",
        ":request",
        ":response",
        ":router",
        "//concurrent:thread_pool",
        "//core:error",
        "//core:log",
        "//core:result",
        "//core:stringify",
        "//http/internal:transform",
        "//net:acceptor",
        "//net:socket",
    ],
)
