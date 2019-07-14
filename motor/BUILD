licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "engine",
    srcs = ["engine.cpp"],
    hdrs = ["engine.h"],
    deps = [
        ":window",
        # Make this a select statement once we have more implementations.
        "//motor/windows:glfw_window",
    ],
)
