
add_library(DirectX-Header INTERFACE)

target_link_libraries(DirectX-Header INTERFACE d3d12.lib)
target_include_directories(DirectX-Header INTERFACE include/directx)