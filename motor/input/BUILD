package(default_visibility = ["//visibility:public"])

cc_library(
    name = "input",
    hdrs = ["input.h"],
    srcs = ["input.cpp"],
    deps = ["//motor:event"],
)

cc_library(
    name = "device",
    hdrs = ["device.h"],
    srcs = ["device.cpp"],
    deps = [
        "//motor:event",
    ],
)
