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

#include "SmartSocket.h"

namespace nkhlab {
namespace sercli {

template <>
bool StartSocket<Server>(SOCKET sock, sockaddr* addr, size_t len)
{
    if (bind(sock, addr, len) != SOCKET_ERROR && listen(sock, SOMAXCONN) != SOCKET_ERROR)
    {
        return true;
    }

    return false;
}

template <>
bool StartSocket<Client>(SOCKET sock, sockaddr* addr, size_t len)
{
    if (connect(sock, addr, len) != SOCKET_ERROR)
    {
        return true;
    }

    return false;
}

// Specialization for Unix

template <>
void SmartSocket<Server, UnixSocket>::Open()
{
    unlink(path_.c_str()); // Remove any existing socket file
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
}

template <>
void SmartSocket<Server, UnixSocket>::Close()
{
    close(sock_);
    unlink(path_.c_str()); // Remove socket file after use
    sock_ = SOCKET_ERROR;
}

template <>
void SmartSocket<Client, UnixSocket>::Open()
{
    sock_ = socket(AF_UNIX, SOCK_STREAM, 0);
}

template <>
void SmartSocket<Client, UnixSocket>::Close()
{
    close(sock_);
    sock_ = SOCKET_ERROR;
}

// Specialization for Inet

template <>
void SmartSocket<Server, InetSocket>::Open()
{
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
}

template <>
void SmartSocket<Server, InetSocket>::Close()
{
    close(sock_);
    sock_ = SOCKET_ERROR;
}

template <>
void SmartSocket<Client, InetSocket>::Open()
{
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
}

template <>
void SmartSocket<Client, InetSocket>::Close()
{
    close(sock_);
    sock_ = SOCKET_ERROR;
}

} // namespace sercli
} // namespace nkhlab