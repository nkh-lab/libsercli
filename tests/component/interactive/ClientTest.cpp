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

#include "CommandHelper.h"
#include "sercli/ClientBuilder.h"

using namespace nkhlab::sercli;
using namespace nkhlab::sercli::tests;

void HandleSendCommand(IClient* client, const std::vector<uint8_t>& data, int nums, int delay_ms)
{
    std::cout << "Sending...\n";
    for (int i = 0; i < nums; ++i)
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

//
// Command format examples:
// q: - quit
// s:<data to send> - send one shot
// s,n<number of sending>,d<delay in ms, between sendings>:<data to send> - send with arguments
//
bool HandleCommand(const std::string& in, IClient* client)
{
    bool ret_quit = false;

    std::string command, data;
    std::map<std::string, int> args;
    bool parsing_res;

    parsing_res = CommandHelper::ParseCommand(in, command, data, args);

    if (parsing_res)
    {
        if (command == "q") // Quit
        {
            ret_quit = true;
        }
        else if (command == "s" && !data.empty()) // Send
        {
            int nums = 1, delay_ms = 0;

            try
            {
                nums = args.at("n");
            }
            catch (...)
            {
            }

            try
            {
                delay_ms = args.at("d");
            }
            catch (...)
            {
            }

            std::vector<uint8_t> bytes(data.begin(), data.end());

            HandleSendCommand(client, bytes, nums, delay_ms);
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
