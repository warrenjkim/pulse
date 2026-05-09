cc_library(
    name = "http",
    visibility = ["//visibility:public"],
    deps = [
        ":method",
        ":request",
        ":response",
    ],
)

cc_library(
    name = "types",
    visibility = ["//visibility:public"],
    deps = [
        ":method",
        ":request",
        ":response",
    ],
)

cc_library(
    name = "method",
    hdrs = ["method.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "request",
    hdrs = ["request.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [":method"],
)

cc_library(
    name = "response",
    hdrs = ["response.h"],
    include_prefix = "pulse/http",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)
