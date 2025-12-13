test_suite(
    name = "tests",
    tests = [
        ":lexer_test",
        ":parser_test",
    ],
)

cc_library(
    name = "token",
    hdrs = [
        "token.h",
    ],
    include_prefix = "pulse/json/parse",
    strip_include_prefix = ".",
    visibility = [
        "//json:__subpackages__",
    ],
)

cc_library(
    name = "reader",
    hdrs = [
        "reader.h",
    ],
    include_prefix = "pulse/json/parse",
    strip_include_prefix = ".",
    visibility = [
        "//json:__subpackages__",
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
    include_prefix = "pulse/json/parse",
    strip_include_prefix = ".",
    visibility = [
        "//json:__subpackages__",
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
        "//json/parse:lexer",
        "//json/parse:token",
        "//json/utils:to_string",
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
    include_prefix = "pulse/json/parse",
    strip_include_prefix = ".",
    visibility = [
        "//json:__subpackages__",
    ],
    deps = [
        ":lexer",
        ":token",
        "//json/utils:exception",
        "//json/utils:to_string",
        "//json/value",
    ],
)

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cc"],
    deps = [
        "//json/parse:lexer",
        "//json/parse:parser",
        "//json/parse:token",
        "//json/utils:exception",
        "@googletest//:gtest_main",
    ],
)
