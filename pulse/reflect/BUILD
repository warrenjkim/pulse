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
    name = "dump_struct",
    hdrs = ["dump_struct.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":data_sink",
    ],
)

cc_test(
    name = "dump_struct_test",
    srcs = ["dump_struct_test.cc"],
    deps = [
        ":data_sink",
        ":dump_struct",
        "//pulse/core:pretty_function",
        "//pulse/core:stringify",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "schema",
    hdrs = ["schema.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "stringify_sink",
    hdrs = ["stringify_sink.h"],
    deps = [
        ":data_sink",
        "//pulse/core:stringify",
    ],
)

cc_library(
    name = "struct_stringify",
    hdrs = ["struct_stringify.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":dump_struct",
        ":stringify_sink",
        "//pulse/core:stringify",
    ],
)

cc_test(
    name = "struct_stringify_test",
    srcs = ["struct_stringify_test.cc"],
    deps = [
        ":struct_stringify",
        "//pulse/core:stringify",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
