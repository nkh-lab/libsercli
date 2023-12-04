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

#include "../src/Macros.h"

#include "libsercli/ServerBuilder.h"

using namespace nkhlab::libsercli;

int main(int argc, char const* argv[])
{
    UNUSED(argc);

    std::cout << "Hello World from " << argv[0] << "\n";

    // Start of libsercli usage code

    auto server = CreateInetServer("127.0.0.1", 12345);

    auto client_status_cb = [](IClientHandlerPtr client, bool connected) {
        printf("Client: %s: %s\n", client->GetId().c_str(), connected ? "connected" : "disconnected");

        if (connected)
        {
            std::string hello("Hello from Server!");
            std::vector<uint8_t> data_to_send(hello.begin(), hello.end());
            client->Send(data_to_send);
        }
    };

    auto server_data_received_cb = [](IClientHandlerPtr client, const std::vector<uint8_t>& data) {
        std::string received_data_str{data.begin(), data.end()};

        printf(
            "Server received from Client: %s: %s\n", client->GetId().c_str(), received_data_str.c_str());
        std::cout.flush();
    };

    server->Start(client_status_cb, server_data_received_cb);

    // End of libsercli usage code

    std::cout << "Press any key + Enter to exit...\n";

    int i;
    std::cin >> i;

    return EXIT_SUCCESS;
}
