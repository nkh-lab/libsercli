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

#ifdef __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#else
#include <BaseTsd.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#endif

#include <string>

namespace nkhlab {
namespace libsercli {

#ifdef __linux__
using SOCKET = int;
constexpr SOCKET kSocketError = -1;
#else
constexpr SOCKET kSocketError = static_cast<SOCKET>(SOCKET_ERROR);
using ssize_t = SSIZE_T;

class WinsockInitializer
{
public:
    static WinsockInitializer& getInstance();

private:
    WinsockInitializer();
    ~WinsockInitializer();
};
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
        : sock_{kSocketError}
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

#ifdef __linux__
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
#endif

class InetSocket : public BaseSocket<sockaddr_in>
{
public:
    InetSocket(const char* address, int port)
    {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);

        sock_addr_.sin_family = AF_INET;
#ifdef __linux__
        sock_addr_.sin_addr.s_addr = inet_addr(address);
        sock_addr_.sin_port = htons(port);
#else
        inet_pton(sock_addr_.sin_family, address, &sock_addr_.sin_addr.s_addr);
        sock_addr_.sin_port = htons(static_cast<u_short>(port));
#endif
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
        if (SocketT::sock_ != kSocketError) Close();
    }

    SmartSocket(const SmartSocket&) = delete;
    SmartSocket& operator=(const SmartSocket&) = delete;

    void Start()
    {
        if (!started_ && SocketT::sock_ != kSocketError)
        {
            if (StartSocket<RoleT>(SocketT::sock_, SocketT::GetAddr(), SocketT::GetLen()))
            {
                started_ = true;
            }
            else
            {
                Close();
            }
        }
    }

#ifdef __linux__
#else
    //
    // to be able to exit from blocking accept()
    //
    void ForceClose()
    {
        Close();
    }
#endif

private:
    void Open();
    void Close();

    bool started_;
};

} // namespace libsercli
} // namespace nkhlab
