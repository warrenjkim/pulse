load("@rules_cc//cc:defs.bzl", _cc_library = "cc_library")
load("//tools/reflect:defs.bzl", "reflect_headers")

def cc_library(name, **kwargs):
    """A `cc_library` that generates reflection for its headers.

    Each `foo.h` gets a generated `foo.reflect.h`. A header with classes marked
    `pulse_reflect` must include it after them:

        #include "path/to/foo.reflect.h"

    Generation runs pulse's own clang, so this works with any C++ toolchain.
    """
    hdrs = kwargs.get("hdrs", [])
    headers = [h for h in hdrs + kwargs.get("srcs", []) if h.endswith(".h")]
    if headers:
        reflect_headers(
            name = name + "_reflect",
            srcs = headers,
            deps = kwargs.get("deps", []),
            testonly = kwargs.get("testonly", False),
        )
        kwargs["hdrs"] = hdrs + [":" + name + "_reflect"]

    _cc_library(name = name, **kwargs)
