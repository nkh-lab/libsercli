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

#include "SocketBuilder.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace nkhlab {
namespace sercli {

int SocketBuilder::InitSocketForUnixServer(const char* unix_socket_path)
{
    return InitSocket(true, true, unix_socket_path, nullptr, -1);
}

int SocketBuilder::InitSocketForInetServer(const char* inet_address, int inet_port)
{
    return InitSocket(true, false, nullptr, inet_address, inet_port);
}

int SocketBuilder::InitSocketForUnixClient(const char* unix_socket_path)
{
    return InitSocket(false, true, unix_socket_path, nullptr, -1);
}

int SocketBuilder::InitSocketForInetClient(const char* inet_address, int inet_port)
{
    return InitSocket(false, false, nullptr, inet_address, inet_port);
}

int SocketBuilder::InitSocket(
    bool is_server,
    bool is_unix,
    const char* unix_socket_path,
    const char* inet_address,
    int inet_port)
{
    int sock = socket((is_unix ? AF_UNIX : AF_INET), SOCK_STREAM, 0);
    if (sock != -1)
    {
        sockaddr_un server_unix_address = {};
        sockaddr_in server_inet_address = {};

        if (is_unix)
        {
            server_unix_address.sun_family = AF_UNIX;
            strncpy(
                server_unix_address.sun_path, unix_socket_path, sizeof(server_unix_address.sun_path));
        }
        else
        {
            server_inet_address.sin_family = AF_INET;
            server_inet_address.sin_addr.s_addr = inet_addr(inet_address);
            server_inet_address.sin_port = htons(inet_port);
        }

        // Difference between Server and Client:
        // - for Server: bind() and listen()
        // - for Client: connect()

        if (is_server)
        {
            if (bind(
                    sock,
                    (is_unix ? reinterpret_cast<sockaddr*>(&server_unix_address)
                             : reinterpret_cast<sockaddr*>(&server_inet_address)),
                    (is_unix ? sizeof(server_unix_address) : sizeof(server_inet_address))) != -1)
            {
                if (listen(sock, SOMAXCONN) == -1)
                {
                    sock = -1;
                }
            }
            else
            {
                sock = -1;
            }
        }
        else
        {
            if (connect(
                    sock,
                    (is_unix ? reinterpret_cast<sockaddr*>(&server_unix_address)
                             : reinterpret_cast<sockaddr*>(&server_inet_address)),
                    (is_unix ? sizeof(server_unix_address) : sizeof(server_inet_address))) == -1)
            {
                sock = -1;
            }
        }
    }

    return sock;
}

} // namespace sercli
} // namespace nkhlab