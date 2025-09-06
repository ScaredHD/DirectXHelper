cmake_minimum_required(VERSION 3.20)

project(DirectXHelper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(isTopLevelProject TRUE)
else()
    set(isTopLevelProject FALSE)
endif()

add_subdirectory(DirectX-Header)

# DX12Helper
file(GLOB_RECURSE helperSource CONFIGURE_DEPENDS DX12Helper/*.cpp)
add_library(DX12Helper ${helperSource})

target_include_directories(
    DX12Helper
    PUBLIC DX12Helper
           DX12Helper/Utils
           DX12Helper/Resources
           DX12Helper/Geometry
           DX12Helper/Render
)

target_link_libraries(
    DX12Helper 
    PUBLIC DirectX-Header
           d3d12.lib
           d3dcompiler.lib
           dxgi.lib
)

target_precompile_headers(
    DX12Helper
    PRIVATE DX12Helper/PCH.h
)


if (isTopLevelProject)
    add_subdirectory(spdlog)
    include(Demo/Utils/DemoUtils.cmake)
    add_subdirectory(Demo/0_SolidColor)
    add_subdirectory(Demo/1_Triangle)
    add_subdirectory(Demo/2_Box)
    add_subdirectory(Demo/3_Instancing)
endif()