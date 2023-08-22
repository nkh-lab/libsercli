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

add_executable(ServerTest ServerTest.cpp)

target_link_libraries(ServerTest
    PRIVATE libsercli
    PRIVATE libcpputils
    )

add_executable(ClientTest ClientTest.cpp)

target_link_libraries(ClientTest
    PRIVATE libsercli
    PRIVATE libcpputils
    )
