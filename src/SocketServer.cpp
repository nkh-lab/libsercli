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

#include "SocketServer.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <algorithm>

namespace nkhlab {
namespace sercli {

SocketClientHandler::SocketClientHandler(int client_socket)
    : client_socket_{client_socket}
    , id_{std::to_string(client_socket)}
    , connected_{true}
{
}

const std::string& SocketClientHandler::SocketClientHandler::GetId()
{
    return id_;
}

bool SocketClientHandler::IsConnected()
{
    return connected_;
}

bool SocketClientHandler::Send(const std::vector<uint8_t>& data)
{
    if (!connected_) return false;

    ssize_t bytes_written = write(client_socket_, data.data(), data.size());

    if (bytes_written == -1 || (static_cast<size_t>(bytes_written) != data.size())) return false;

    return true;
}

void SocketClientHandler::Disconnected()
{
    connected_ = false;
}

SocketServer::SocketServer(const std::string& unix_socket_path)
    : is_unix_{true}
    , unix_socket_path_{unix_socket_path}
    , stopped_{true}
{
    unlink(unix_socket_path_.c_str()); // Remove any existing socket file
}

SocketServer::SocketServer(const std::string& inet_address, int port)
    : is_unix_{false}
    , stopped_{true}
{
}

SocketServer::~SocketServer()
{
    Stop();
}

bool SocketServer::Start(ClientStatusCb client_status_cb, ServerDataReceivedCb server_data_received_cb)
{
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1)
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

    if (bind(
            server_socket,
            reinterpret_cast<sockaddr*>(&server_unix_address),
            sizeof(server_unix_address)) == -1)
    {
        // Handle error
        return false;
    }

    if (listen(server_socket, SOMAXCONN) == -1)
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

    stopped_ = false;
    worker_thread_ =
        std::thread([&, server_socket, epoll_fd, client_status_cb, server_data_received_cb]() {
            epoll_event server_event;
            server_event.data.fd = server_socket;
            server_event.events = EPOLLIN;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &server_event);
            constexpr int MAX_EVENTS = 10; // TODO: why?
            constexpr int STOP_HANDLE_TIMEOUT_MS = 500;
            std::vector<epoll_event> events(MAX_EVENTS);

            while (!stopped_)
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
                    int client_socket;

                    if (events[i].data.fd == server_socket)
                    {
                        // New client connected
                        client_socket = accept(server_socket, nullptr, nullptr);
                        epoll_event client_event;
                        client_event.data.fd = client_socket;
                        client_event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
                        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event);

                        auto client = AddClient(client_socket);

                        if (client && client_status_cb) client_status_cb(client, true);
                    }
                    else
                    {
                        // Handle data from existing clients
                        client_socket = events[i].data.fd;
                        constexpr size_t RECEIVER_BUFFER_SIZE = 1024; // TODO: configurable?
                        std::vector<uint8_t> buffer(RECEIVER_BUFFER_SIZE);

                        int bytes_read = read(client_socket, buffer.data(), RECEIVER_BUFFER_SIZE);
                        if (bytes_read == 0)
                        {
                            // Client disconnected
                            auto client = GetClient(client_socket);

                            if (client)
                            {
                                client->Disconnected();
                                RemoveClient(client_socket);
                                if (client_status_cb) client_status_cb(client, false);
                            }

                            // Handle the disconnection
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                            close(client_socket);
                        }
                        else if (bytes_read < 0)
                        {
                            // Error occurred
                        }
                        else
                        {
                            // Handle received data
                            if (server_data_received_cb)
                            {
                                auto client = GetClient(client_socket);
                                buffer.resize(bytes_read);
                                server_data_received_cb(client, buffer);
                            }
                        }
                    }
                }
            }

            close(server_socket);
            close(epoll_fd);
            unlink(unix_socket_path_.c_str()); // Remove socket file
        });

    return true;
}

void SocketServer::Stop()
{
    if (!stopped_)
    {
        stopped_ = true;

        if (worker_thread_.joinable()) worker_thread_.join();
    }
}

std::vector<IClientHandlerPtr> SocketServer::GetClients()
{
    std::lock_guard<std::mutex> lk(clients_mtx_);

    std::vector<IClientHandlerPtr> clients;

    std::transform(clients_.begin(), clients_.end(), std::back_inserter(clients), [](auto& kv) {
        return kv.second;
    });

    return clients;
}

IClientHandlerPtr SocketServer::GetClient(const std::string& id)
{
    int id_int;
    IClientHandlerPtr client = nullptr;

    try
    {
        id_int = static_cast<int>(std::stoi(id));

        client = GetClient(id_int);
    }
    catch (...)
    {
    }

    return client;
}

SocketClientHandlerPtr SocketServer::GetClient(int id)
{
    std::lock_guard<std::mutex> lk(clients_mtx_);

    auto it = clients_.find(id);

    if (it != clients_.end())
        return (*it).second;
    else
        return nullptr;
}

SocketClientHandlerPtr SocketServer::AddClient(int id)
{
    std::lock_guard<std::mutex> lk(clients_mtx_);

    auto client = std::make_shared<SocketClientHandler>(id);

    auto emplace_res = clients_.emplace(id, client);

    if (emplace_res.second)
        return client;
    else
        return nullptr;
}

void SocketServer::RemoveClient(int id)
{
    std::lock_guard<std::mutex> lk(clients_mtx_);
    clients_.erase(id);
}

} // namespace sercli
} // namespace nkhlab
