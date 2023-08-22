#
# Copyright (C) 2023 https://github.com/nkh-lab
#
# This is free software. You can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 3 as published by the Free Software Foundation.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.
#
file(GLOB SRC_FILES "src/*.cpp")

add_library(libcpputils STATIC
    ${SRC_FILES}
    )

target_include_directories(libcpputils
    PUBLIC include
    )