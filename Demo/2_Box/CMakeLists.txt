
add_executable(DemoBox WIN32 Box.cpp)

target_link_libraries(DemoBox PRIVATE DX12Helper)

SetupDemoOutput(DemoBox TRUE)