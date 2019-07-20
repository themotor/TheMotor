licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "engine",
    srcs = ["engine.cpp"],
    hdrs = ["engine.h"],
    deps = [
        ":event",
        ":window",
        "//motor/input:input",
        # Make this a select statement once we have more implementations.
        "//motor/windows:glfw_window",
        "@glog//:glog",
    ],
)

cc_library(
    name = "window",
    hdrs = ["window.h"],
    srcs = ["window.cpp"],
    deps = [
        ":event",
    ],
)

cc_library(
    name = "event",
    hdrs = ["event.h"],
    srcs = ["event.cpp"],
)
