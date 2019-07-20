licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "glfw_window",
    srcs = ["glfw_window.cpp"],
    deps = [
        "@glfw//:glfw",
        "@glog//:glog",
        "//motor:event",
        "//motor:window",
        "//motor/input:input",
    ],
    alwayslink = 1,
)
