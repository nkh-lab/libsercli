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

#include <algorithm>
#include <iostream>
#include <sstream>
#include <thread>

#include "CommandHelper.h"
#include "libsercli/ClientBuilder.h"

using namespace nkhlab::libsercli;
using namespace nkhlab::libsercli::tests;

void HandleSendCommand(IClient* client, const std::vector<uint8_t>& data, int nums, int delay_ms)
{
    std::cout << "Sending...\n";
    for (int i = 0; i < nums; ++i)
    {
        if (!client->Send(data))
        {
            std::cout << "Sending Failed!\n";
            return;
        }
        if (delay_ms > 0 && i < nums - 1)
            std::this_thread::sleep_for(std::chrono::milliseconds{delay_ms});
    }
    std::cout << "Sending Done!\n";
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

    nkhlab::libsercli::IClientPtr client;

    if (argc < 2)
    {
        std::cout << "No configuration provided! Please provide it as arguments.\n";
        std::cout << "For UNIX socket connection: <unix socket path>\n";
        std::cout << "For Inet connection:        <inet address> <inet port>\n";
        return EXIT_FAILURE;
    }

    if (argc == 2)
    {
        const char* socket_path = argv[1];

        client = CreateUnixClient(socket_path);
    }
    else if (argc == 3)
    {
        const char* inet_address = argv[1];
        int inet_port = atoi(argv[2]);

        client = CreateInetClient(inet_address, inet_port);
    }
    else
    {
        std::cout << "Incorrect use of arguments. Please use as below:\n";
        std::cout << "For UNIX socket connection: <unix socket path>\n";
        std::cout << "For Inet connection:        <inet address> <inet port>\n";
        return EXIT_FAILURE;
    }

    if (!client)
    {
        std::cout << "ERROR: client is nullptr!\n";
        return EXIT_FAILURE;
    }

    ServerDisconnectedCb server_disconnected_cb = []() { std::cout << "Server disconnected\n"; };
    ClientDataReceivedCb client_data_received_cb = [](const std::vector<uint8_t>& data) {
        std::string data_str{data.begin(), data.end()};

        std::cout << "Server sent data: " << data_str << "\n";
    };

    if (client->Connect(server_disconnected_cb, client_data_received_cb))
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
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
