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

#include <iostream>

#include "sercli/ServerBuilder.h"

using namespace nkhlab::sercli;

int main(int argc, char const* argv[])
{
    std::cout << "Hello World from SocketServerTest!\n";

    auto server = CreateUnixServer("/tmp/my_unix_socket");

    ClientStatusCb client_status_cb = [](IClientHandlerPtr client, bool connected) {
        if (connected)
        {
            std::cout << "Client with ID: " << client->GetId() << " connected\n";

            client->SubscribeToReceive([](IClientHandlerPtr client, const std::vector<uint8_t>& data) {
                std::string data_str{data.begin(), data.end()};

                std::cout << "Client with ID: " << client->GetId() << " sent data: " << data_str
                          << "\n";
            });
        }
        else
            std::cout << "Client with ID: " << client->GetId() << " diconnected\n";
    };

    if (server)
    {
        server->Start(client_status_cb);

        int in;
        std::cin >> in;

        server->Stop();
    }
    else
    {
        std::cout << "ERROR: server is nullptr!\n";
    }

    return 0;
}
