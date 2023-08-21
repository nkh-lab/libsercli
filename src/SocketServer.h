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

#include <atomic>
#include <map>
#include <thread>
#include <mutex>

#include "sercli/IServer.h"

namespace nkhlab {
namespace sercli {

class SocketClientHandler
    : public IClientHandler
    , public std::enable_shared_from_this<SocketClientHandler>
{
public:
    SocketClientHandler(int client_socket);
    ~SocketClientHandler() = default;

    const std::string& GetId() override;

    bool IsConnected() override;
    bool Send(const std::vector<uint8_t>& data) override;
    bool SubscribeToReceive(ServerDataReceivedCb data_received_cb) override;
    void Disconnected();
    void OnReceive(const std::vector<uint8_t>& data);

private:
    std::mutex mutex_;
    int client_socket_;
    std::string id_;
    bool connected_;
    ServerDataReceivedCb data_received_cb_;
};

using SocketClientHandlerPtr = std::shared_ptr<SocketClientHandler>;

class SocketServer : public IServer
{
public:
    SocketServer(const std::string& unix_socket);
    SocketServer(const std::string& inet_address, int port);

    ~SocketServer();

    bool Start(ClientStatusCb client_status_cb) override;
    void Stop() override;

private:
    bool is_unix_;
    std::string unix_socket_path_;
    std::thread worker_thread_;
    std::atomic_bool stopped_;

    std::map<int, SocketClientHandlerPtr> clients_;
};

} // namespace sercli
} // namespace nkhlab