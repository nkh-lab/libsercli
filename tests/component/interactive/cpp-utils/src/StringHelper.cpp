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

#include "cpp-utils/StringHelper.h"

#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdarg.h>

namespace nkhlab {
namespace cpputils {

//
// Split a string into strings by delimiter
//
std::vector<std::string> StringHelper::SplitStr(
    const std::string& input,
    const std::string& delimiter,
    bool only_first,
    bool skip_empty)
{
    std::vector<std::string> strings;
    size_t start = 0, end = 0;
    std::string sub_str;

    while ((end = input.find(delimiter, start)) != std::string::npos)
    {
        sub_str = input.substr(start, end - start);
        start = end + delimiter.length();

        (skip_empty && sub_str.size() == 0) ? void() : strings.push_back(sub_str);

        if (only_first) break;
    }

    // Add the last substring after the last delimiter
    sub_str = input.substr(start);
    (skip_empty && sub_str.size() == 0) ? void() : strings.push_back(sub_str);

    return strings;
}

} // namespace cpputils
} // namespace nkhlab
