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

#include <condition_variable>
#include <iostream>
#include <thread>

#include "sercli/ClientBuilder.h"
#include "sercli/ServerBuilder.h"

using namespace nkhlab::sercli;
using namespace std::chrono_literals;

constexpr char kHandshakeRequest[] = "Hello Client!";
constexpr char kHandshakeReply[] = "Hello Server!";

int main(int argc, char const* argv[])
{
    std::condition_variable cv;
    std::mutex cv_m;
    bool cv_ready = false;
    int cv_data = EXIT_FAILURE;

    std::cout << "Hello World from HandshakeTest!\n";

    if (argc < 2)
    {
        std::cout << "No socket path provided! Please provide it as argument.\n";
        return EXIT_FAILURE;
    }

    std::string socket_path(argv[1]);

    auto server = CreateUnixServer(socket_path);

    ClientStatusCb client_status_cb = [&](IClientHandlerPtr client, bool connected) {
        if (connected)
        {
            std::cout << "Client with ID: " << client->GetId() << " connected\n";

            client->SubscribeToReceive([&](IClientHandlerPtr client, const std::vector<uint8_t>& data) {
                std::string data_str{data.begin(), data.end()};

                std::cout << "Client with ID: " << client->GetId() << " sent data: " << data_str
                          << "\n";
                {
                    std::lock_guard<std::mutex> lk(cv_m);
                    cv_ready = true;
                    cv_data = EXIT_SUCCESS;
                }
                cv.notify_all();
            });

            std::string request(kHandshakeRequest);
            std::vector<uint8_t> request_bytes(request.begin(), request.end());

            client->Send(request_bytes);
        }
        else
            std::cout << "Client with ID: " << client->GetId() << " diconnected\n";
    };

    if (server)
    {
        if (server->Start(client_status_cb))
        {
            std::this_thread::sleep_for(100ms);

            auto client = CreateUnixClient(socket_path);

            if (client)
            {
                ServerDisconnectedCb server_disconnected_cb = []() {
                    std::cout << "Server disconnected\n";
                };
                ClientDataReceivedCb client_data_received_cb = [&](const std::vector<uint8_t>& data) {
                    std::string data_str{data.begin(), data.end()};

                    std::cout << "Client received data: " << data_str << "\n";

                    if (data_str == kHandshakeRequest)
                    {
                        std::string reply(kHandshakeReply);
                        std::vector<uint8_t> reply_bytes(reply.begin(), reply.end());

                        client->Send(reply_bytes);
                    }
                };

                if (client->Connect(server_disconnected_cb, client_data_received_cb))
                {
                    std::unique_lock<std::mutex> lk(cv_m);
                    if (cv.wait_for(lk, 5s, [&]() { return cv_ready; }))
                    {
                        std::cout << "Successfull handshaking!\n";
                    }
                    else
                    {
                        std::cout << "ERROR: Timeout reached no client reply!\n";
                    }

                    return cv_data;
                }
                else
                {
                    std::cout << "ERROR: client failed on connect!\n";
                    return EXIT_FAILURE;
                }
            }
            else
            {
                std::cout << "ERROR: client is nullptr!\n";
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cout << "ERROR: server failed on start!\n";
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cout << "ERROR: server is nullptr!\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
