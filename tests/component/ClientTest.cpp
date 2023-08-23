/*
 * Copyright (C) 2022 https://github.com/nkh-lab
 *
 * This is free software. You can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <thread>

#include "cpp-utils/StringHelper.h"
#include "sercli/ClientBuilder.h"

using namespace nkhlab::cpputils;
using namespace nkhlab::sercli;

void HandleSendCommand(IClient* client, const std::vector<uint8_t>& data, size_t nums, size_t delay_ms)
{
    std::cout << "Sending...\n";
    for (size_t i = 0; i < nums; ++i)
    {
        if (!client->Send(data))
        {
            std::cout << "Failed!\n";
            return;
        }
        if (delay_ms > 0 && i < nums - 1)
            std::this_thread::sleep_for(std::chrono::milliseconds{delay_ms});
    }
    std::cout << "Done!\n";
}

void RetrieveNumsAndDelayFromSendCommand(const std::string& send_command, size_t& ret_nums, size_t& ret_delay_ms)
{
    ret_nums = 1;
    ret_delay_ms = 0;

    std::vector<std::string> sub_commands = StringHelper::SplitStr(send_command, ",");

    for (auto s : sub_commands)
    {
        if (*s.begin() == 'n') ret_nums = std::stoi(s.substr(1));
        if (*s.begin() == 'd') ret_delay_ms = std::stoi(s.substr(1));
    }
}

//
// Command format
// q
// q:
// s:<data to send>
// s,n<number of sending>,d<delay in ms, between sendings>:<data to send>
//
bool HandleCommand(const std::string& in, IClient* client)
{
    bool ret_quit = false;

    std::vector<std::string> command_data = StringHelper::SplitStr(in, ":");

    if (command_data.size() > 0)
    {
        if (command_data[0] == "q") // Quit
        {
            ret_quit = true;
        }
        else if (command_data[0][0] == 's') // Send
        {
            if (command_data[1].size() > 0)
            {
                std::vector<uint8_t> data(command_data[1].begin(), command_data[1].end());

                size_t nums;
                size_t delay_ms;

                RetrieveNumsAndDelayFromSendCommand(command_data[0], nums, delay_ms);

                HandleSendCommand(client, data, nums, delay_ms);
            }
        }
    }

    return ret_quit;
}

int main(int argc, char const* argv[])
{
    std::cout << "Hello World from SocketClientTest!\n";

    auto client = CreateUnixClient("/tmp/my_unix_socket");

    ServerDisconnectedCb server_disconnected_cb = []() { std::cout << "Server disconnected\n"; };

    if (client->Connect(server_disconnected_cb, nullptr))
    {
        for (;;)
        {
            std::string in;

            std::getline(std::cin, in);

            if (HandleCommand(in, client.get())) break;
        }

        client->Disconnect();
    }
    else
    {
        std::cout << "Connection to Server failed!\n";
    }

    return 0;
}
