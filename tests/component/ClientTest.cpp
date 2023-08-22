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

#include "cpp-utils/StringHelper.h"
#include "sercli/ClientBuilder.h"

using namespace nkhlab::cpputils;
using namespace nkhlab::sercli;

//
// Command format
// q
// q:
// s:<data to send>
// s,n10,t100ms:<data to send>
//
bool HandleCommand(const std::string& in, IClient* client)
{
    bool ret_quit = false;

    std::vector<std::string> command_data = StringHelper::SplitStr(in, ":");

    if (command_data.size() > 0)
    {
        if (command_data[0] == "q")
            ret_quit = true; // Quit
        else
        {
            std::vector<std::string> commands = StringHelper::SplitStr(command_data[0], ",");

            if (commands[0] == "s") // Send
            {
                if (commands.size() == 1)
                {
                    std::vector<uint8_t> data(command_data[1].begin(), command_data[1].end());
                    client->Send(data);
                }
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
        std::cout << "Connection to Server FAILED!\n";
    }

    return 0;
}
