

add_executable(DemoSolidColor WIN32 SolidColorWindow.cpp)

target_link_libraries(DemoSolidColor PRIVATE DX12Helper)

SetupDemoOutput(DemoSolidColor FALSE)

