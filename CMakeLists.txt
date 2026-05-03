cmake_minimum_required(VERSION 3.31.6)
project(cs515_ciphers)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cs515_ciphers ciphershell.cpp)

if (MSVC)
    set_property(TARGET cs515_ciphers PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif (WIN32)
    target_link_options(cs515_ciphers PRIVATE -static -static-libgcc -static-libstdc++)
endif()
