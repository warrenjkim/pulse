cc_library(
    name = "blocking_queue",
    hdrs = ["blocking_queue.h"],
    include_prefix = "pulse/concurrent",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)

cc_test(
    name = "blocking_queue_test",
    srcs = ["blocking_queue_test.cc"],
    deps = [
        ":blocking_queue",
        "@googletest//:gtest_main",
    ],
)
