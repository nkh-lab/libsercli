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

#include <iostream>
#include <thread>

#include "CommandHelper.h"
#include "libsercli/ServerBuilder.h"

using namespace nkhlab::libsercli;
using namespace nkhlab::libsercli::tests;

void HandleSendCommand(IServer* server, const std::vector<uint8_t>& data, int client_id, int nums, int delay_ms)
{
    std::cout << "Sending...\n";

    if (client_id == -1)
    {
        if (!server->GetClients().empty())
        {
            for (int i = 0; i < nums; ++i)
            {
                if (!server->GetClients().empty())
                {
                    for (auto c : server->GetClients())
                    {
                        c->Send(data);
                    }
                    if (delay_ms > 0 && i < nums - 1)
                        std::this_thread::sleep_for(std::chrono::milliseconds{delay_ms});
                }
                else
                {
                    std::cout << "Sending Failed!\n";
                    return;
                }
            }
        }
        else
        {
            std::cout << "Sending Failed!\n";
            return;
        }
    }
    else
    {
        auto c = server->GetClient(std::to_string(client_id));

        if (c)
        {
            for (int i = 0; i < nums; ++i)
            {
                c->Send(data);
                if (delay_ms > 0 && i < nums - 1)
                    std::this_thread::sleep_for(std::chrono::milliseconds{delay_ms});
            }
        }
        else
        {
            std::cout << "Sending Failed!\n";
            return;
        }
    }
    std::cout << "Sending Done!\n";
}

//
// Command format examples:
// q: - quit
// s:<data to send> - send one shot
// s,n<number of sending>,d<delay in ms, between sendings>:<data to send> - send with arguments
//
bool HandleCommand(const std::string& in, IServer* server)
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
            int nums = 1, delay_ms = 0, client_id = -1;

            try
            {
                client_id = args.at("c");
            }
            catch (...)
            {
            }

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

            HandleSendCommand(server, bytes, client_id, nums, delay_ms);
        }
    }

    return ret_quit;
}

int main(int argc, char const* argv[])
{
    std::cout << "Hello World from SocketServerTest!\n";

    nkhlab::libsercli::IServerPtr server;

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

        server = CreateUnixServer(socket_path);
    }
    else if (argc == 3)
    {
        const char* inet_address = argv[1];
        int inet_port = atoi(argv[2]);

        server = CreateInetServer(inet_address, inet_port);
    }
    else
    {
        std::cout << "Incorrect use of arguments. Please use as below:\n";
        std::cout << "For UNIX socket connection: <unix socket path>\n";
        std::cout << "For Inet connection:        <inet address> <inet port>\n";
        return EXIT_FAILURE;
    }

    if (!server)
    {
        std::cout << "ERROR: server is nullptr!\n";
        return EXIT_FAILURE;
    }

    ClientStatusCb client_status_cb = [&](IClientHandlerPtr client, bool connected) {
        if (connected)
            std::cout << "Client with ID: " << client->GetId() << " connected\n";
        else
            std::cout << "Client with ID: " << client->GetId() << " diconnected\n";

        std::cout << "Total clients: " << server->GetClients().size() << "\n";
    };

    ServerDataReceivedCb server_data_received_cb =
        ([](IClientHandlerPtr client, const std::vector<uint8_t>& data) {
            std::string data_str{data.begin(), data.end()};

            std::cout << "Client with ID: " << client->GetId() << " sent data: " << data_str << "\n";
        });

    server->Start(client_status_cb, server_data_received_cb);

    for (;;)
    {
        std::string in;

        std::getline(std::cin, in);

        if (HandleCommand(in, server.get())) break;
    }

    server->Stop();

    return EXIT_SUCCESS;
}
