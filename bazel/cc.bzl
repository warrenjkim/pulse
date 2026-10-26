load(
    "@rules_cc//cc:defs.bzl",
    _cc_binary = "cc_binary",
    _cc_library = "cc_library",
    _cc_test = "cc_test",
)

_PLUGIN = str(Label("//tools/reflect:plugin"))

_COMPILE_ATTRS = [
    "copts",
    "defines",
    "includes",
    "local_defines",
]

def _with_plugin(kwargs):
    kwargs["copts"] = kwargs.get("copts", []) + [
        "-fplugin=$(location %s)" % _PLUGIN,
        "-DPULSE_REFLECT_PLUGIN=1",
    ]
    kwargs["additional_compiler_inputs"] = kwargs.get(
        "additional_compiler_inputs",
        [],
    ) + [_PLUGIN]
    return kwargs

def _sources_library(name, srcs, deps, testonly, kwargs):
    lib = {
        "name": name + "_lib",
        "srcs": srcs,
        "deps": deps,
        "testonly": testonly,
        "alwayslink": True,
    }
    for attr in _COMPILE_ATTRS:
        if attr in kwargs:
            lib[attr] = kwargs.pop(attr)

    cc_library(**lib)
    return [":" + name + "_lib"]

def cc_library(**kwargs):
    _cc_library(**_with_plugin(kwargs))

def cc_test(name, srcs = [], deps = [], **kwargs):
    _cc_test(
        name = name,
        deps = _sources_library(name, srcs, deps, True, kwargs),
        **kwargs
    )

def cc_binary(name, srcs = [], deps = [], **kwargs):
    _cc_binary(
        name = name,
        deps = _sources_library(
            name,
            srcs,
            deps,
            kwargs.get("testonly", False),
            kwargs,
        ),
        **kwargs
    )
