# STARS UI Workspace

workspace(name = "stars_ui")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Python Rules
http_archive(
    name = "rules_python",
    sha256 = "690e0141724ac108c3c3f42584400408f19f93e8f7ef0fb108ab04a5e1a8f1e3",
    strip_prefix = "rules_python-0.31.0",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.31.0/rules_python-0.31.0.tar.gz",
)

load("@rules_python//python:repositories.bzl", "py_repositories", "python_register_toolchains")

py_repositories()

python_register_toolchains(
    name = "python3_11",
    python_version = "3.11",
)

load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "pip_deps",
    requirements_lock = "//examples:requirements.txt",
)

load("@pip_deps//:requirements.bzl", "install_deps")

install_deps()
