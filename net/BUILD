cc_library(
    name = "net",
    visibility = ["//visibility:public"],
    deps = [
        ":acceptor",
        ":socket",
    ],
)

cc_library(
    name = "acceptor",
    srcs = ["acceptor.cc"],
    hdrs = ["acceptor.h"],
    include_prefix = "pulse/net",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [":socket"],
)

cc_library(
    name = "socket",
    srcs = ["socket.cc"],
    hdrs = ["socket.h"],
    include_prefix = "pulse/net",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
)
