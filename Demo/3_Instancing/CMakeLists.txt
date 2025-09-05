
add_executable(DemoInstancing WIN32 Boxes.cpp Instances.cpp Culling.cpp)

target_link_libraries(DemoInstancing PRIVATE DX12Helper)

SetupDemoOutput(DemoInstancing TRUE)

target_link_libraries(DemoInstancing PRIVATE spdlog)