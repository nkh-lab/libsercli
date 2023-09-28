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

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <string>

namespace nkhlab {
namespace sercli {

#ifdef __linux__
using SOCKET = int;
constexpr int SOCKET_ERROR = -1;
#endif

// RoleT
class Server;
class Client;

// SockT
class UnixSocket;
class InetSocket;

template <class RoleT>
bool StartSocket(SOCKET sock, sockaddr* addr, size_t len);

template <class SockAddrT>
class BaseSocket
{
public:
    BaseSocket()
        : sock_{SOCKET_ERROR}
    {
    }
    BaseSocket(const BaseSocket&) = delete;
    BaseSocket& operator=(const BaseSocket&) = delete;

    SOCKET GetRawSocket() { return sock_; }

protected:
    sockaddr* GetAddr() { return reinterpret_cast<sockaddr*>(&sock_addr_); }

    size_t GetLen() { return sizeof(SockAddrT); }

    SOCKET sock_;
    SockAddrT sock_addr_;
};

class UnixSocket : public BaseSocket<sockaddr_un>
{
public:
    UnixSocket(const char* path)
        : path_{path}
    {
        sock_addr_.sun_family = AF_UNIX;
        strncpy(sock_addr_.sun_path, path_.c_str(), sizeof(sock_addr_.sun_path));
    }

protected:
    const std::string path_;
};

class InetSocket : public BaseSocket<sockaddr_in>
{
public:
    InetSocket(const char* address, int port)
    {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);

        sock_addr_.sin_family = AF_INET;
        sock_addr_.sin_addr.s_addr = inet_addr(address);
        sock_addr_.sin_port = htons(port);
    }
};

template <class RoleT, class SocketT>
class SmartSocket : public SocketT
{
public:
    template <class... Args>
    SmartSocket(const Args&... args)
        : SocketT(args...)
        , started_{false}
    {
        Open();
    }

    ~SmartSocket()
    {
        if (SocketT::sock_ != SOCKET_ERROR) Close();
    }

    SmartSocket(const SmartSocket&) = delete;
    SmartSocket& operator=(const SmartSocket&) = delete;

    void Start()
    {
        if (!started_ && SocketT::sock_ != SOCKET_ERROR)
        {
            if (StartSocket<RoleT>(SocketT::sock_, SocketT::GetAddr(), SocketT::GetLen()))
            {
                started_ = true;
            }
            {
                Close();
            }
        }
    }

private:
    void Open();
    void Close();

    bool started_;
};

} // namespace sercli
} // namespace nkhlab
