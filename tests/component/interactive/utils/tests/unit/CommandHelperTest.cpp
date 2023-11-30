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

#include <gtest/gtest.h>

#include "CommandHelper.h"

using namespace nkhlab::sercli::tests;

TEST(CommandHelperTest, ParseGoodCommands)
{
    std::string command, data;
    std::map<std::string, int> args;
    bool parsing_res;

    parsing_res = CommandHelper::ParseCommand("s:data_to_send", command, data, args);

    EXPECT_TRUE(parsing_res);
    EXPECT_STREQ("s", command.c_str());
    EXPECT_STREQ("data_to_send", data.c_str());
    EXPECT_TRUE(args.empty());

    parsing_res = CommandHelper::ParseCommand("s,n10,d1000:data_to_send", command, data, args);

    EXPECT_TRUE(parsing_res);
    EXPECT_STREQ("s", command.c_str());
    EXPECT_STREQ("data_to_send", data.c_str());
    EXPECT_EQ(args.size(), 2);
    if (args.size() == 2)
    {
        EXPECT_EQ(args["n"], 10);
        EXPECT_EQ(args["d"], 1000);
    }

    parsing_res =
        CommandHelper::ParseCommand("send,num10,delay1000:data_to_send", command, data, args);

    EXPECT_TRUE(parsing_res);
    EXPECT_STREQ("send", command.c_str());
    EXPECT_STREQ("data_to_send", data.c_str());
    EXPECT_EQ(args.size(), 2);
    if (args.size() == 2)
    {
        EXPECT_EQ(args["num"], 10);
        EXPECT_EQ(args["delay"], 1000);
    }

    parsing_res = CommandHelper::ParseCommand("q:", command, data, args);

    EXPECT_TRUE(parsing_res);
    EXPECT_STREQ("q", command.c_str());
    EXPECT_TRUE(data.empty());
    EXPECT_TRUE(args.empty());
}

TEST(CommandHelperTest, ParseBadCommands)
{
    std::string command, data;
    std::map<std::string, int> args;
    bool parsing_res;

    parsing_res = CommandHelper::ParseCommand("s,10:data_to_send", command, data, args);

    EXPECT_FALSE(parsing_res);
    EXPECT_TRUE(command.empty());
    EXPECT_TRUE(data.empty());
    EXPECT_TRUE(args.empty());

    parsing_res = CommandHelper::ParseCommand("s,,:data_to_send", command, data, args);

    EXPECT_FALSE(parsing_res);
    EXPECT_TRUE(command.empty());
    EXPECT_TRUE(data.empty());
    EXPECT_TRUE(args.empty());

    parsing_res = CommandHelper::ParseCommand("q", command, data, args);

    EXPECT_FALSE(parsing_res);
    EXPECT_TRUE(command.empty());
    EXPECT_TRUE(data.empty());
    EXPECT_TRUE(args.empty());
}