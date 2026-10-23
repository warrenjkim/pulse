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
