/*
 * Copyright (C) 2023 https://github.com/nkh-lab
 *
 * This is free software. You can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 */

#pragma once

#include <string>
#include <vector>

namespace nkhlab {
namespace cpputils {

class StringHelper
{
public:
    StringHelper() = delete;

    //
    // Split a string into strings by delimiter
    //
    static std::vector<std::string> SplitStr(
        const std::string& input,
        const std::string& delimiter,
        bool skip_empty = true);
};

} // namespace cpputils
} // namespace nkhlab