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

#include "sercli/ClientBuilder.h"

using namespace nkhlab::sercli;

int main(int argc, char const* argv[])
{
    std::cout << "Hello World from SocketClientTest!\n";

    auto client = CreateUnixClient("/tmp/my_unix_socket");

    ServerDisconnectedCb server_disconnected_cb = []() { std::cout << "Server disconnected\n"; };

    if (client->Connect(server_disconnected_cb, nullptr))
    {
        int in;
        std::cin >> in;

        client->Disconnect();
    }
    else
    {
        std::cout << "Connection to Server FAILED!\n";
    }

    return 0;
}