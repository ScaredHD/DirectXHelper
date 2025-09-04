
add_executable(DemoInstancing WIN32 Boxes.cpp Instances.cpp)

target_link_libraries(DemoInstancing PRIVATE DX12Helper)

SetupDemoOutput(DemoInstancing TRUE)