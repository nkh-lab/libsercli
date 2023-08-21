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

#include <sys/socket.h>
#include <sys/un.h>

#include <atomic>
#include <map>
#include <thread>

#include "sercli/IClient.h"

namespace nkhlab {
namespace sercli {

class SocketClient : public IClient
{
public:
    SocketClient(const std::string& unix_socket);
    SocketClient(const std::string& inet_address, int port);

    bool Connect(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb) override;
    void Disconnect() override;

    void Send(const std::vector<uint8_t>& data) override;

private:
    bool is_unix_;
    std::string unix_socket_path_;
    int client_socket_;
};

} // namespace sercli
} // namespace nkhlab