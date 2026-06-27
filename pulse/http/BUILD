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
    visibility = ["//visibility:public"],
    deps = ["//pulse/core:enum_macros"],
)

cc_library(
    name = "pattern",
    srcs = ["pattern.cc"],
    hdrs = ["pattern.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":parameters",
        ":url",
        "//pulse/core:enum_macros",
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/core:stringify",
        "//pulse/strings:cat",
        "//pulse/strings:split",
    ],
)

cc_test(
    name = "pattern_test",
    srcs = ["pattern_test.cc"],
    deps = [
        ":pattern",
        "//pulse/core:error",
        "//pulse/core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "request",
    hdrs = ["request.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":method",
        ":parameters",
        "//pulse/core:stringify",
        "//pulse/strings:cat",
    ],
)

cc_library(
    name = "response",
    hdrs = ["response.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:stringify",
        "//pulse/strings:cat",
    ],
)

cc_library(
    name = "router",
    srcs = ["router.cc"],
    hdrs = ["router.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handler",
        ":method",
        ":pattern",
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/core:stringify",
        "//pulse/strings:cat",
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
        "//pulse/core:error",
        "//pulse/core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "handler",
    hdrs = ["handler.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":request",
        ":response",
        "//pulse/core:type_map",
        "//pulse/strings:string_literal",
    ],
)

cc_library(
    name = "server",
    srcs = ["server.cc"],
    hdrs = ["server.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handler",
        ":method",
        ":request",
        ":response",
        ":router",
        "//pulse/concurrent:thread_pool",
        "//pulse/core:error",
        "//pulse/core:log",
        "//pulse/core:result",
        "//pulse/core:stringify",
        "//pulse/http/internal:cors",
        "//pulse/http/internal:transform",
        "//pulse/net:acceptor",
        "//pulse/net:socket",
    ],
)

cc_library(
    name = "parameters",
    hdrs = ["parameters.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":parameter_parser",
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/core:stringify",
        "//pulse/strings:cat",
    ],
)

cc_test(
    name = "parameters_test",
    srcs = ["parameters_test.cc"],
    deps = [
        ":parameters",
        "//pulse/core:error",
        "//pulse/core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "parameter_parser",
    hdrs = ["parameter_parser.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:error",
        "//pulse/core:result",
    ],
)

cc_library(
    name = "parse_form",
    srcs = ["parse_form.cc"],
    hdrs = ["parse_form.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":parameters",
        ":url",
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/strings:cat",
    ],
)

cc_test(
    name = "parse_form_test",
    srcs = ["parse_form_test.cc"],
    deps = [
        ":parameters",
        ":parse_form",
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/strings:cat",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "url",
    srcs = ["url.cc"],
    hdrs = ["url.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/strings:cat",
    ],
)

cc_test(
    name = "url_test",
    srcs = ["url_test.cc"],
    deps = [
        ":url",
        "//pulse/core:error",
        "//pulse/core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
