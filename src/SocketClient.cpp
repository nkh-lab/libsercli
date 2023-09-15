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

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "SocketBuilder.h"

namespace nkhlab {
namespace sercli {

SocketClient::SocketClient(const std::string& unix_socket_path)
    : is_unix_{true}
    , unix_socket_path_{unix_socket_path}
    , inet_address_{}
    , inet_port_{-1}
    , disconnected_{true}
{
}

SocketClient::SocketClient(const std::string& inet_address, int inet_port)
    : is_unix_{false}
    , unix_socket_path_{}
    , inet_address_{inet_address}
    , inet_port_{inet_port}
    , disconnected_{true}
{
}

SocketClient::~SocketClient()
{
    Disconnect();
}

bool SocketClient::Connect(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb)
{
    Disconnect();

    client_socket_ =
        (is_unix_ ? SocketBuilder::InitSocketForUnixClient(unix_socket_path_.c_str())
                  : SocketBuilder::InitSocketForInetClient(inet_address_.c_str(), inet_port_));

    if (client_socket_ == -1)
    {
        // Handle error
        return false;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        // Handle error
        return false;
    }

    disconnected_ = false;
    worker_thread_ = std::thread([&, epoll_fd, server_disconnected_cb, data_received_cb]() {
        epoll_event client_event;
        client_event.data.fd = client_socket_;
        client_event.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_, &client_event);
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
                if (events[i].data.fd == client_socket_)
                {
                    // Handle data from Server
                    constexpr size_t RECEIVER_BUFFER_SIZE = 1024; // TODO: configurable?
                    std::vector<uint8_t> buffer(RECEIVER_BUFFER_SIZE);

                    int bytes_read = read(client_socket_, buffer.data(), RECEIVER_BUFFER_SIZE);
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

        close(client_socket_);
        close(epoll_fd);
    });

    return true;
}

void SocketClient::Disconnect()
{
    disconnected_ = true;

    if (worker_thread_.joinable()) worker_thread_.join();
}

bool SocketClient::Send(const std::vector<uint8_t>& data)
{
    if (disconnected_) return false;

    // Send the message to the server using write()
    ssize_t bytes_written = write(client_socket_, data.data(), data.size());

    if (bytes_written == -1 || (static_cast<size_t>(bytes_written) != data.size())) return false;

    return true;
}

} // namespace sercli
} // namespace nkhlab
