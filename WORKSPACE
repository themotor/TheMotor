workspace(
    name = "TheMotor",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "glfw",
    build_file = "BUILD.glfw",
    url = "https://github.com/glfw/glfw/archive/3.3.tar.gz",
    strip_prefix = "glfw-3.3",
    sha256 = "81bf5fde487676a8af55cb317830703086bb534c53968d71936e7b48ee5a0f3e",
)
http_archive(
    name = "glog",
    url = "https://github.com/google/glog/archive/v0.4.0.tar.gz",
    strip_prefix = "glog-0.4.0",
    sha256 = "f28359aeba12f30d73d9e4711ef356dc842886968112162bc73002645139c39c",
)

http_archive(
    name = "com_github_gflags_gflags",
    strip_prefix = "gflags-2.2.2",
    urls = [
        "https://mirror.bazel.build/github.com/gflags/gflags/archive/v2.2.2.tar.gz",
        "https://github.com/gflags/gflags/archive/v2.2.2.tar.gz",
    ],
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
)

http_archive(
    name = "vulkan",
    url = "https://github.com/KhronosGroup/Vulkan-Headers/archive/v1.1.115.tar.gz",
    strip_prefix = "Vulkan-Headers-1.1.115",
    sha256 = "9298c9a591ecbfbe399b659eac2ae0ee8845601235859a741f38ced1a8144fe3",
    build_file = "BUILD.vulkan",
)
