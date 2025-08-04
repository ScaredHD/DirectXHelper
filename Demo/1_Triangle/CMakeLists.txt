

add_executable(DemoTriangle WIN32 Triangle.cpp)
target_link_libraries(DemoTriangle PRIVATE DX12Helper)

SetupDemoOutput(DemoTriangle TRUE)