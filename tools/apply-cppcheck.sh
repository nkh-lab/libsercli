#!/bin/bash

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

# -I - include
# -i - ignore
# --check-config - use it only for check configuration, e.g. what includes are missed

cppcheck ./ --inconclusive --enable=all --suppress=missingIncludeSystem \
        -I appmy/src \
        -I build/libconfigmy \
        -I libmy/include \
        -I libmy/src \
        -I libmy/tests/unit/mock \
        -I libutilsmy/include \
        -i build \
        -i external \
        -i tools
        