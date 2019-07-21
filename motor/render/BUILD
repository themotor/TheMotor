package(default_visibility = ["//visibility:public"])

cc_library(
    name = "renderer",
    srcs = ["renderer.cpp"],
    hdrs = ["renderer.h"],
    deps = [
        "//motor:plugin",
    ],
)

cc_library(
    name = "vulkan_renderer",
    srcs = ["vulkan_renderer.cpp"],
    deps = [
        ":renderer",
        "@glog//:glog",
        "@glfw//:glfw",
        "@vulkan//:vulkan",
    ],
    alwayslink = 1,
)
