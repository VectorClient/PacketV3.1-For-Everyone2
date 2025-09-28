cmake_minimum_required(VERSION 3.29)
project(loader CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(dependencies)
link_libraries(minhook)
include_directories(loader/ interceptor/ includes/)

# Loader
add_library(loader SHARED
    loader/resource/resource.rc
    loader/dllmain.cc    
    loader/loader.cc
    loader/scanner.cc
    loader/files.cc
    loader/kiero/Kiero.cc
)
target_link_libraries(loader LightningScanner)

# Interceptor
add_library(interceptor SHARED
    interceptor/dllmain.cc
    interceptor/hook.cc
)
target_link_libraries(interceptor LightningScanner)
