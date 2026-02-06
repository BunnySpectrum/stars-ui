load("@rules_cc//cc:defs.bzl", "cc_library")

# nanosvg needs NANOSVG_IMPLEMENTATION defined in exactly one translation unit.
# We create a source file that does this.
genrule(
    name = "nanosvg_impl",
    outs = ["nanosvg_impl.cpp"],
    cmd = """
echo '#define NANOSVG_IMPLEMENTATION' > $@
echo '#define NANOSVGRAST_IMPLEMENTATION' >> $@
echo '#include "nanosvg.h"' >> $@
echo '#include "nanosvgrast.h"' >> $@
""",
)

cc_library(
    name = "nanosvg",
    srcs = [":nanosvg_impl"],
    hdrs = ["src/nanosvg.h", "src/nanosvgrast.h"],
    includes = ["src"],
    visibility = ["//visibility:public"],
)
