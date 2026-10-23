load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load(":defs.bzl", "clang_verify_test", "exec_file")

_PLUGIN_COPTS = [
    "-std=c++23",
    "-fno-rtti",
]

_PLUGIN_FEATURES = [
    "-layering_check",
    "-parse_headers",
]

exports_files([
    "clang_verify_test.sh",
    "testdata/mirror_stub.h",
])

cc_library(
    name = "attribute",
    srcs = ["attribute.cc"],
    hdrs = ["attribute.h"],
    copts = _PLUGIN_COPTS,
    features = _PLUGIN_FEATURES,
    deps = ["//third_party/llvm:clang_headers"],
)

clang_verify_test(
    name = "attribute_test",
    src = "attribute_test.cc",
)

cc_library(
    name = "consumer",
    srcs = ["consumer.cc"],
    hdrs = ["consumer.h"],
    copts = _PLUGIN_COPTS,
    features = _PLUGIN_FEATURES,
    deps = [
        ":attribute",
        ":extract",
        ":inject",
        ":record",
        "//third_party/llvm:clang_headers",
    ],
)

cc_library(
    name = "action",
    srcs = ["action.cc"],
    hdrs = ["action.h"],
    copts = _PLUGIN_COPTS,
    features = _PLUGIN_FEATURES,
    deps = [
        ":consumer",
        "//third_party/llvm:clang_headers",
    ],
)

cc_library(
    name = "extract",
    srcs = ["extract.cc"],
    hdrs = ["extract.h"],
    copts = _PLUGIN_COPTS,
    features = _PLUGIN_FEATURES,
    deps = [
        ":record",
        "//third_party/llvm:clang_headers",
    ],
)

clang_verify_test(
    name = "extract_test",
    src = "extract_test.cc",
)

cc_library(
    name = "record",
    hdrs = ["record.h"],
)

cc_library(
    name = "source_writer",
    hdrs = ["source_writer.h"],
)

cc_test(
    name = "source_writer_test",
    srcs = ["source_writer_test.cc"],
    deps = [
        ":source_writer",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "render",
    srcs = ["render.cc"],
    hdrs = ["render.h"],
    copts = _PLUGIN_COPTS,
    deps = [
        ":record",
        ":source_writer",
    ],
)

cc_test(
    name = "render_test",
    srcs = ["render_test.cc"],
    deps = [
        ":record",
        ":render",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "inject",
    srcs = ["inject.cc"],
    hdrs = ["inject.h"],
    copts = _PLUGIN_COPTS,
    features = _PLUGIN_FEATURES,
    deps = ["//third_party/llvm:clang_headers"],
)

clang_verify_test(
    name = "inject_test",
    src = "inject_test.cc",
)

cc_binary(
    name = "pulse_reflect.so",
    srcs = ["plugin.cc"],
    copts = _PLUGIN_COPTS,
    features = _PLUGIN_FEATURES,
    linkopts = select({
        "@platforms//os:macos": [
            "-undefined",
            "dynamic_lookup",
        ],
        "//conditions:default": ["-Wl,-z,undefs"],
    }),
    linkshared = True,
    deps = [
        ":action",
        ":attribute",
        "//third_party/llvm:clang_headers",
    ],
)

exec_file(
    name = "plugin",
    src = ":pulse_reflect.so",
    visibility = ["//visibility:public"],
)
