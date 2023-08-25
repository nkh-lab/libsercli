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

#include "CommandHelper.h"

#include "cpp-utils/StringHelper.h"

using namespace nkhlab::cpputils;

namespace nkhlab {
namespace sercli {
namespace tests {

bool CommandHelper::ParseCommand(
    const std::string& in,
    std::string& command,
    std::string& data,
    std::map<std::string, int>& args)
{
    bool ret = true;

    command.clear();
    data.clear();
    args.clear();

    if (in.find(":") != std::string::npos)
    {
        auto command_data = StringHelper::SplitStr(in, ":", true);

        if (command_data.size() > 0 && command_data.size() < 3)
        {
            auto command_args = StringHelper::SplitStr(in, ",", false, false);
            if (command_args.size() > 1)
            {
                for (size_t i = 0; i < command_args.size(); ++i)
                {
                    if (i == 0)
                        command = command_args[i];
                    else
                    {
                        std::string arg;
                        int value;
                        if (ParseArgument(command_args[i], arg, value))
                        {
                            args.emplace(arg, value);
                        }
                        else
                        {
                            ret = false;
                            break;
                        }
                    }
                }
            }
            else
                command = command_data[0];

            if (command_data.size() > 1) data = command_data[1];
        }
        else
            ret = false;
    }
    else
        ret = false;

    if (!ret)
    {
        command.clear();
        data.clear();
        args.clear();
    }

    return ret;
}

bool CommandHelper::ParseArgument(const std::string& in, std::string& arg, int& value)
{
    bool ret = true;

    if (!in.empty())
    {
        std::size_t found = in.find_first_of("0123456789");

        if (found != std::string::npos)
        {
            if (found > 0)
            {
                arg = in.substr(0, found);
                try
                {
                    value = static_cast<int>(std::stoi(in.substr(found)));
                }
                catch (const std::exception& e)
                {
                    ret = false;
                }
            }
            else
            {
                ret = false;
            }
        }
        else
        {
            arg = in;
        }
    }
    else
        ret = false;

    return ret;
}

} // namespace tests
} // namespace sercli
} // namespace nkhlab