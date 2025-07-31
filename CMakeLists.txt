cmake_minimum_required(VERSION 3.20)

project(DirectXHelper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(DirectX-Header)

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