config_setting(
    name = "macos_aarch64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:aarch64",
    ],
)

config_setting(
    name = "linux_aarch64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:aarch64",
    ],
)

config_setting(
    name = "linux_x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

alias(
    name = "clang_headers",
    actual = select({
        ":macos_aarch64": "@llvm_macos_aarch64//:clang_headers",
        ":linux_aarch64": "@llvm_linux_aarch64//:clang_headers",
        ":linux_x86_64": "@llvm_linux_x86_64//:clang_headers",
    }),
    visibility = ["//tools/reflect:__pkg__"],
)
