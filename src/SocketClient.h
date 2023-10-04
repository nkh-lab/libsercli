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

#include "SmartSocket.h"

namespace nkhlab {
namespace sercli {

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

        Disconnect();

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
        constexpr int STOP_HANDLE_TIMEOUT_MS = 500;
        std::vector<epoll_event> events(MAX_EVENTS);

        while (!disconnected_)
        {
            int num_events = epoll_wait(
                epoll_fd,
                events.data(),
                MAX_EVENTS,
                STOP_HANDLE_TIMEOUT_MS); // if pass __timeout as -1 - no timeout
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
                    constexpr size_t RECEIVER_BUFFER_SIZE = 1024; // TODO: configurable?
                    std::vector<uint8_t> buffer(RECEIVER_BUFFER_SIZE);

                    int bytes_read =
                        read(client_socket_.GetRawSocket(), buffer.data(), RECEIVER_BUFFER_SIZE);
                    if (bytes_read == 0)
                    {
                        // Server disconnected
                        if (server_disconnected_cb) server_disconnected_cb();
                        disconnected_ = true;
                        break;
                        break;
                    }
                    else if (bytes_read < 0)
                    {
                        // Error occurred
                    }
                    else
                    {
                        // Handle received data
                        if (data_received_cb)
                        {
                            buffer.resize(bytes_read);
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
    }
#endif

    SmartSocket<Client, SocketT> client_socket_;
    std::thread worker_thread_;
    std::atomic_bool disconnected_;
};

} // namespace sercli
} // namespace nkhlab
