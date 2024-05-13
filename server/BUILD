cc_binary(
    name = "main",
    # "All headers that belong to the binary or test directly should be listed in the srcs": https://bazel.build/reference/be/c-cpp#:~:text=All%20headers%20that%20belong%20to%20the%20binary%20or%20test%20directly%20should%20be%20listed%20in%20the%20srcs.
    srcs = ["main.cpp", "main.hpp", "commands.cpp", "commands.hpp"],
    deps = ['@nlohmann_json//:json'],
    data = ['//data:test.json']
)

