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

#pragma once

#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <atomic>
#include <map>
#include <thread>

#include "sercli/IClient.h"

#include "Constants.h"
#include "SmartSocket.h"

namespace nkhlab {
namespace sercli {

constexpr int kStopHandleTimeout_ms = 500;

template <class SocketT>
class SocketClient : public IClient
{
public:
    template <class... Args>
    SocketClient(const Args&... args)
        : client_socket_{args...}
        , disconnected_{true}
    {
    }

    ~SocketClient() { Disconnect(); }

    bool Connect(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb) override
    {
        bool ret = false;

        client_socket_.Start();

        if (client_socket_.GetRawSocket() != kSocketError)
        {
            disconnected_ = false;
            worker_thread_ =
                std::thread(&SocketClient::Routine, this, server_disconnected_cb, data_received_cb);
            ret = true;
        }

        return ret;
    }

    void Disconnect() override
    {
        disconnected_ = true;
#ifdef __linux__
#else
        client_socket_.ForceClose();
#endif
        if (worker_thread_.joinable()) worker_thread_.join();
    }

    bool Send(const std::vector<uint8_t>& data) override
    {
        if (disconnected_) return false;

#ifdef __linux__
        ssize_t bytes_written = write(client_socket_.GetRawSocket(), data.data(), data.size());
#else
        ssize_t bytes_written = send(
            client_socket_.GetRawSocket(),
            reinterpret_cast<const char*>(data.data()),
            static_cast<int>(data.size()),
            0);
#endif
        if (bytes_written == -1 || bytes_written != static_cast<ssize_t>(data.size())) return false;

        return true;
    }

private:
#ifdef __linux__
    void Routine(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb)
    {
        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            // Handle error
            disconnected_ = true;
        }

        epoll_event client_event;
        client_event.data.fd = client_socket_.GetRawSocket();
        client_event.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_.GetRawSocket(), &client_event);
        constexpr int MAX_EVENTS = 10; // TODO: why?
        std::vector<epoll_event> events(MAX_EVENTS);

        while (!disconnected_)
        {
            int num_events = epoll_wait(
                epoll_fd,
                events.data(),
                MAX_EVENTS,
                kStopHandleTimeout_ms); // if pass __timeout as -1 - no timeout
            if (num_events == -1)
            {
                // Handle error
                break;
            }
            else if (num_events == 0)
            {
                // Timeout occurred, no events - we use it to handle stop request
            }

            for (int i = 0; i < num_events; ++i)
            {
                if (events[i].data.fd == client_socket_.GetRawSocket())
                {
                    // Handle data from Server
                    std::vector<uint8_t> buffer(kDataBufferSize);

                    int received_bytes =
                        read(client_socket_.GetRawSocket(), buffer.data(), buffer.size());
                    if (received_bytes == 0)
                    {
                        // Server disconnected
                        if (server_disconnected_cb) server_disconnected_cb();
                        disconnected_ = true;
                        break;
                        break;
                    }
                    else if (received_bytes < 0)
                    {
                        // Error occurred
                    }
                    else
                    {
                        // Handle received data
                        if (data_received_cb)
                        {
                            buffer.resize(received_bytes);
                            data_received_cb(buffer);
                        }
                    }
                }
            }
        }

        if (epoll_fd != -1) close(epoll_fd);
    }
#else
    void Routine(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb)
    {
        WSAOVERLAPPED overlapped = {};
        overlapped.hEvent = WSACreateEvent();

        if (overlapped.hEvent != nullptr)
        {
            while (!disconnected_)
            {
                SOCKET sock = client_socket_.GetRawSocket();
                std::vector<uint8_t> buffer(kDataBufferSize);
                WSABUF wsa_buf;
                wsa_buf.len = static_cast<ULONG>(buffer.size());
                wsa_buf.buf = reinterpret_cast<char*>(buffer.data());
                DWORD flags = 0;
                DWORD received_bytes = 0;

                if (overlapped.hEvent != nullptr)
                {
                    if (WSARecv(sock, &wsa_buf, 1, &received_bytes, &flags, &overlapped, nullptr) ==
                            SOCKET_ERROR &&
                        WSAGetLastError() != WSA_IO_PENDING)
                    {
                        if (server_disconnected_cb) server_disconnected_cb();
                        disconnected_ = true;
                    }
                    else
                    {
                        if (WSAWaitForMultipleEvents(1, &overlapped.hEvent, TRUE, INFINITE, TRUE) !=
                            WSA_WAIT_FAILED)
                        {
                            if (WSAGetOverlappedResult(
                                    sock, &overlapped, &received_bytes, FALSE, &flags))
                            {
                                if (received_bytes > 0)
                                {
                                    if (data_received_cb)
                                    {
                                        buffer.resize(received_bytes);
                                        data_received_cb(buffer);
                                    }
                                }

                                WSAResetEvent(overlapped.hEvent);
                            }
                        }
                    }
                }
            }
            WSACloseEvent(overlapped.hEvent);
        }
    }
#endif

    SmartSocket<Client, SocketT> client_socket_;
    std::thread worker_thread_;
    std::atomic_bool disconnected_;
};

} // namespace sercli
} // namespace nkhlab
