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

#include <map>
#include <string>

namespace nkhlab {
namespace sercli {
namespace tests {

//
// To handle commands from cin:
// <command>:<data> - q: or s:Hello!
// <command>,<arg><int value>:<data> - s,n10,d1000:ping
//
class CommandHelper
{
public:
    CommandHelper() = delete;

    static bool ParseCommand(
        const std::string& in,
        std::string& command,
        std::string& data,
        std::map<std::string, int>& args);

private:
    static bool ParseArgument(const std::string& in, std::string& arg, int& value);
};

} // namespace tests
} // namespace sercli
} // namespace nkhlab