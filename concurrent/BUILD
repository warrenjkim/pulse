cc_library(
    name = "concurrent",
    visibility = ["//visibility:public"],
    deps = [
        ":blocking_queue",
        ":thread_pool",
    ],
)

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

cc_library(
    name = "thread_pool",
    srcs = ["thread_pool.cc"],
    hdrs = ["thread_pool.h"],
    include_prefix = "pulse/concurrent",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [":blocking_queue"],
)

cc_test(
    name = "thread_pool_test",
    srcs = ["thread_pool_test.cc"],
    deps = [
        ":thread_pool",
        "@googletest//:gtest_main",
    ],
)
