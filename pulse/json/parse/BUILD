cc_library(
    name = "token",
    hdrs = [
        "token.h",
    ],
    visibility = [
        "//pulse/json:__subpackages__",
    ],
)

cc_library(
    name = "reader",
    hdrs = [
        "reader.h",
    ],
    visibility = [
        "//pulse/json:__subpackages__",
    ],
)

cc_library(
    name = "lexer",
    srcs = [
        "lexer.cc",
    ],
    hdrs = [
        "lexer.h",
    ],
    visibility = [
        "//pulse/json:__subpackages__",
    ],
    deps = [
        ":reader",
        ":token",
    ],
)

cc_test(
    name = "lexer_test",
    srcs = ["lexer_test.cc"],
    deps = [
        "//pulse/json/parse:lexer",
        "//pulse/json/parse:token",
        "//pulse/json/utils:to_string",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "parser",
    srcs = [
        "parser.cc",
    ],
    hdrs = [
        "parser.h",
    ],
    visibility = [
        "//pulse/json:__subpackages__",
    ],
    deps = [
        ":lexer",
        ":token",
        "//pulse/json/utils:exception",
        "//pulse/json/utils:to_string",
        "//pulse/json/value",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    deps = [
        ":lexer",
        ":parser",
        ":token",
        "//pulse/json/utils:exception",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
