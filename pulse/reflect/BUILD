cc_library(
    name = "reflect",
    visibility = ["//visibility:public"],
    deps = [
        ":data_sink",
        ":schema",
    ],
)

cc_library(
    name = "data_sink",
    hdrs = ["data_sink.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:stringifiable_type",
    ],
)

cc_library(
    name = "schema",
    hdrs = ["schema.h"],
    visibility = ["//visibility:public"],
)
