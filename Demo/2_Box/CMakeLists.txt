
add_executable(DemoBox Box.cpp)

target_link_libraries(DemoBox PRIVATE DirectXHelper)

SetupDemoOutput(DemoBox TRUE)