cmake_minimum_required(VERSION 3.20)

project(DirectXHelper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(DirectX-Header)

# Dear ImGui
set(imgui_dir ${PROJECT_SOURCE_DIR}/imgui)

file(GLOB imgui_src ${imgui_dir}/*.cpp)
set( imgui_backends
    ${imgui_dir}/backends/imgui_impl_dx12.cpp
    ${imgui_dir}/backends/imgui_impl_win32.cpp
)

add_library(imgui 
    ${imgui_src} 
    ${imgui_backends}
)

target_include_directories(imgui 
    PUBLIC ${imgui_dir} 
           ${imgui_dir}/backends
)

# GraphicsToolkit
add_subdirectory(${PROJECT_SOURCE_DIR}/External/GraphicsToolkit)

# DX12Helper
file(GLOB_RECURSE helperSource DX12Helper/*.cpp)
add_library(DX12Helper ${helperSource})

target_link_libraries(
    DX12Helper 
    PUBLIC DirectX-Header
           imgui
           Gtk
           d3d12.lib
           d3dcompiler.lib
           dxgi.lib
           tinyobjloader
)

target_include_directories(
    DX12Helper
    PUBLIC DX12Helper
)


# TestApp
add_executable(
    TestApp 
    Test/test_app.cpp
)

target_link_libraries(TestApp DX12Helper)