cc_library(
    name = "html",
    visibility = ["//visibility:public"],
    deps = [
        ":attributes",
        ":tags",
        ":text",
    ],
)

cc_library(
    name = "attributes",
    hdrs = ["attributes.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "tags",
    hdrs = ["tags.h"],
    visibility = ["//visibility:public"],
    deps = [":attributes"],
)

cc_library(
    name = "text",
    hdrs = ["text.h"],
    visibility = ["//visibility:public"],
)
