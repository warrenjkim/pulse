cc_library(
    name = "core",
    visibility = ["//visibility:public"],
    deps = [
        ":enum_macros",
        ":stringify",
    ],
)

cc_library(
    name = "enum_macros",
    hdrs = ["enum_macros.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "stringify",
    hdrs = ["stringify.h"],
    include_prefix = "pulse/core",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)
