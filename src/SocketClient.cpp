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

#include "SocketClient.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace nkhlab {
namespace sercli {

SocketClient::SocketClient(const std::string& unix_socket_path)
    : is_unix_{true}
    , unix_socket_path_{unix_socket_path}
{
}

SocketClient::SocketClient(const std::string& inet_address, int port)
    : is_unix_{false}
{
}

bool SocketClient::Connect(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb)
{
    client_socket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket_ == -1)
    {
        // Handle error
        return false;
    }

    sockaddr_un server_unix_address;
    server_unix_address.sun_family = AF_UNIX;
    strncpy(
        server_unix_address.sun_path, unix_socket_path_.c_str(), sizeof(server_unix_address.sun_path));

    // Difference between Server and Client:
    // - for Server: bind() and listen()
    // - for Client: connect()

    if (connect(
            client_socket_,
            reinterpret_cast<sockaddr*>(&server_unix_address),
            sizeof(server_unix_address)) == -1)
    {
        // Handle error
        return false;
    }

    return true;
}

void SocketClient::Disconnect()
{
    close(client_socket_);
}

void SocketClient::Send(const std::vector<uint8_t>& data)
{
}

} // namespace sercli
} // namespace nkhlab