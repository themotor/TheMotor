licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "engine",
    srcs = ["engine.cpp"],
    hdrs = ["engine.h"],
    deps = [
        ":event",
        ":plugin",
        ":window",
        "//motor/input:input",
        "//motor/render:renderer",
        # Make this a select statement once we have more implementations.
        "//motor/windows:glfw_window",
        "//motor/render:vulkan_renderer",
        "@glog//:glog",
    ],
)

cc_library(
    name = "window",
    hdrs = ["window.h"],
    srcs = ["window.cpp"],
    deps = [
        ":event",
        ":plugin",
    ],
)

cc_library(
    name = "event",
    hdrs = ["event.h"],
    srcs = ["event.cpp"],
)

cc_library(
    name = "plugin",
    hdrs = ["plugin.h"],
    srcs = ["plugin.cpp"],
)
