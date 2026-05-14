cc_library(
    name = "http",
    visibility = ["//visibility:public"],
    deps = [
        ":handler",
        ":method",
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
    name = "request",
    hdrs = ["request.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        ":method",
        "//core:stringify",
    ],
)

cc_library(
    name = "response",
    hdrs = ["response.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = ["//core:stringify"],
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
        ":method",
        "//concurrent:thread_pool",
        "//core:stringify",
        "//dsa:result",
        "//http/internal:transform",
        "//net:acceptor",
        "//net:socket",
    ],
)
