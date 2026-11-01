load("@rules_cc//cc:action_names.bzl", "CPP_COMPILE_ACTION_NAME")
load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain", "use_cc_toolchain")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")
load("@rules_shell//shell:sh_test.bzl", "sh_test")

_CLANG = str(Label("//third_party/llvm:clang"))
_PLUGIN = str(Label("//tools/reflect:plugin"))
_VERIFY = str(Label("//tools/reflect:clang_verify_test.sh"))
_STUB = str(Label("//tools/reflect:testdata/mirror_stub.h"))

def _exec_file_impl(ctx):
    return [DefaultInfo(files = depset([ctx.file.src]))]

exec_file = rule(
    implementation = _exec_file_impl,
    attrs = {
        "src": attr.label(allow_single_file = True, cfg = "exec"),
    },
)

def _reflect_headers_impl(ctx):
    cc_toolchain = find_cc_toolchain(ctx)
    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features + [
            "layering_check",
            "module_maps",
            "parse_headers",
        ],
    )
    compilation_context = cc_common.merge_compilation_contexts(
        compilation_contexts = [dep[CcInfo].compilation_context for dep in ctx.attr.deps],
    )

    # NOTE: Each header includes its generated header, which does not exist
    # while it is being generated. Empty stubs stand in for them.
    stubs = []
    outputs = []
    for src in ctx.files.srcs:
        name = src.basename.removesuffix(".h") + ".reflect.h"
        outputs.append(ctx.actions.declare_file(name, sibling = src))
        stub = ctx.actions.declare_file("_%s_stubs/%s" % (ctx.label.name, outputs[-1].short_path))
        ctx.actions.write(stub, "")
        stubs.append(stub)

    stub_root = stubs[0].path.removesuffix("/" + outputs[0].short_path) if stubs else ""
    plugin = ctx.file._plugin
    inputs = depset(
        direct = ctx.files.srcs + stubs + [plugin],
        transitive = [compilation_context.headers, cc_toolchain.all_files],
    )
    compiler = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = CPP_COMPILE_ACTION_NAME,
    )

    for src, output in zip(ctx.files.srcs, outputs):
        variables = cc_common.create_compile_variables(
            feature_configuration = feature_configuration,
            cc_toolchain = cc_toolchain,
            source_file = src.path,
            user_compile_flags = ctx.fragments.cpp.copts + ctx.fragments.cpp.cxxopts + [
                "-x",
                "c++-header",
                "-fsyntax-only",
                # NOTE: The toolchain passes `-c`, which `-fsyntax-only` ignores.
                "-Wno-unused-command-line-argument",
                "-fplugin=" + plugin.path,
                "-fplugin-arg-pulse_reflect-emit=" + output.path,
                "-fplugin-arg-pulse_reflect-include=" + output.short_path,
            ],
            include_directories = compilation_context.includes,
            quote_include_directories = depset(
                [stub_root, ".", ctx.bin_dir.path],
                transitive = [compilation_context.quote_includes],
            ),
            system_include_directories = compilation_context.system_includes,
            framework_include_directories = compilation_context.framework_includes,
            preprocessor_defines = compilation_context.defines,
        )
        ctx.actions.run(
            executable = compiler,
            arguments = cc_common.get_memory_inefficient_command_line(
                feature_configuration = feature_configuration,
                action_name = CPP_COMPILE_ACTION_NAME,
                variables = variables,
            ),
            env = cc_common.get_environment_variables(
                feature_configuration = feature_configuration,
                action_name = CPP_COMPILE_ACTION_NAME,
                variables = variables,
            ),
            inputs = inputs,
            outputs = [output],
            mnemonic = "PulseReflect",
            progress_message = "Generating reflection for " + src.short_path,
        )

    return [DefaultInfo(files = depset(outputs))]

# Generates `foo.reflect.h` next to each `foo.h` in `srcs` with the
# pulse_reflect plugin.
reflect_headers = rule(
    implementation = _reflect_headers_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".h"]),
        "deps": attr.label_list(providers = [CcInfo]),
        "_plugin": attr.label(default = _PLUGIN, allow_single_file = True),
    },
    fragments = ["cpp"],
    toolchains = use_cc_toolchain(),
)

def clang_verify_test(name, src, args = [], data = []):
    sh_test(
        name = name,
        srcs = [_VERIFY],
        args = [
            "$(rootpath %s)" % _CLANG,
            "$(rootpath %s)" % _PLUGIN,
            "$(rootpath %s)" % _STUB,
            "$(rootpath %s)" % src,
        ],
        # NOTE: Not `args`, which also get each `--test_arg`.
        env = {"CLANG_VERIFY_ARGS": " ".join(args)},
        data = data + [
            src,
            _CLANG,
            _PLUGIN,
            _STUB,
        ],
    )
