licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "glfw_window",
    srcs = ["glfw_window.cpp"],
    deps = [
        "//motor:window",
        "@glfw//:glfw",
        "//motor:event",
    ],
    alwayslink = 1,
)
