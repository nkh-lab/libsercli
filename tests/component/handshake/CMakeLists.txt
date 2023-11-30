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

add_executable(HandshakeTest HandshakeTest.cpp)

target_link_libraries(HandshakeTest
    PRIVATE libsercli
    )
