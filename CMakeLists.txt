#
# Copyright (C) 2022 https://github.com/nkh-lab
#
# This is free software. You can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 3 as published by the Free Software Foundation.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.
#

cmake_minimum_required(VERSION 3.4.1)

project(libsercli)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(MSVC)
    add_compile_options(/W4 /WX)
    list(APPEND OS_LIBS Ws2_32)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

option(${PROJECT_NAME}_BUILD_UTESTS     "Build unit tests"          OFF)
option(${PROJECT_NAME}_BUILD_CTESTS     "Build component tests"     OFF)
option(${PROJECT_NAME}_BUILD_PORTABLE   "Build portable binaries"   OFF)

# Interface Library

add_library(${PROJECT_NAME}-headers INTERFACE)

target_include_directories(${PROJECT_NAME}-headers INTERFACE include)

# Implementation Library

file(GLOB SRC_FILES "src/*.cpp")

add_library(${PROJECT_NAME} SHARED
    ${SRC_FILES}
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PRIVATE src
    )

target_link_libraries(${PROJECT_NAME}
    PRIVATE ${OS_LIBS}
    )

add_subdirectory(tests)