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
#include <mutex>
#include <thread>

#include "sercli/IServer.h"

namespace nkhlab {
namespace sercli {

class SocketClientHandler : public IClientHandler
{
public:
    SocketClientHandler(int client_socket);
    ~SocketClientHandler() = default;

    const std::string& GetId() override;

    bool IsConnected() override;
    bool Send(const std::vector<uint8_t>& data) override;
    void Disconnected();

private:
    const int client_socket_;
    const std::string id_;
    std::atomic_bool connected_;
};

using SocketClientHandlerPtr = std::shared_ptr<SocketClientHandler>;

class SocketServer : public IServer
{
public:
    SocketServer(const std::string& unix_socket_path);
    SocketServer(const std::string& inet_address, int port);

    ~SocketServer();

    bool Start(ClientStatusCb client_status_cb, ServerDataReceivedCb server_data_received_cb) override;
    void Stop() override;

private:
    const bool is_unix_;
    const std::string unix_socket_path_;
    std::thread worker_thread_;
    std::atomic_bool stopped_;

    std::map<int, SocketClientHandlerPtr> clients_;
};

} // namespace sercli
} // namespace nkhlab