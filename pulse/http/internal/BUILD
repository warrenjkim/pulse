cc_library(
    name = "cors",
    srcs = ["cors.cc"],
    hdrs = ["cors.h"],
    visibility = ["//pulse/http:__pkg__"],
    deps = [
        "//pulse/http:request",
        "//pulse/http:response",
    ],
)

cc_test(
    name = "cors_test",
    srcs = ["cors_test.cc"],
    deps = [
        ":cors",
        "//pulse/http:request",
        "//pulse/http:response",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
    visibility = ["//pulse/http:__pkg__"],
    deps = [
        "//pulse/core:error",
        "//pulse/core:log",
        "//pulse/core:result",
        "//pulse/core:stringify",
        "//pulse/http:method",
        "//pulse/http:request",
        "//pulse/http:response",
        "//pulse/strings:cat",
        "//pulse/strings:split",
    ],
)

cc_test(
    name = "transform_test",
    srcs = ["transform_test.cc"],
    deps = [
        ":transform",
        "//pulse/core:result",
        "//pulse/http:method",
        "//pulse/http:request",
        "//pulse/http:response",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
