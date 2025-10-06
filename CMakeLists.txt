cmake_minimum_required(VERSION 3.29)
project(project CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LOADER_STANDALONE_MODE OFF)

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
    loader/remapper.cc
    loader/kiero/Kiero.cc
)
if(LOADER_STANDALONE_MODE)
    target_compile_definitions(loader PRIVATE DLL_STANDALONE)
endif()
target_link_libraries(loader LightningScanner)

# Interceptor
add_library(interceptor SHARED
    interceptor/dllmain.cc
    interceptor/hook.cc
)
target_link_libraries(interceptor LightningScanner)

# Launcher
add_executable(launcher
    launcher/main.cc
    launcher/injector.cc
)