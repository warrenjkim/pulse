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
    visibility = ["//visibility:public"],
    deps = [
        ":socket",
        "//pulse/core:log",
    ],
)

cc_library(
    name = "socket",
    srcs = ["socket.cc"],
    hdrs = ["socket.h"],
    visibility = ["//visibility:public"],
    deps = ["//pulse/core:log"],
)
