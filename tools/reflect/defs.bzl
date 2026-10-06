load("@rules_shell//shell:sh_test.bzl", "sh_test")

_CLANG = str(Label("@llvm_toolchain_llvm//:bin/clang"))
_PLUGIN = str(Label("//tools/reflect:plugin"))
_VERIFY = str(Label("//tools/reflect:clang_verify_test.sh"))

def _exec_file_impl(ctx):
    return [DefaultInfo(files = depset([ctx.file.src]))]

exec_file = rule(
    implementation = _exec_file_impl,
    attrs = {
        "src": attr.label(allow_single_file = True, cfg = "exec"),
    },
)

def clang_verify_test(name, src):
    sh_test(
        name = name,
        srcs = [_VERIFY],
        args = [
            "$(rootpath %s)" % _CLANG,
            "$(rootpath %s)" % _PLUGIN,
            "$(rootpath %s)" % src,
        ],
        data = [
            src,
            _CLANG,
            _PLUGIN,
        ],
    )
