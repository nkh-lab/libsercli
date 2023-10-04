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

#ifdef __linux__
    ssize_t bytes_written = write(client_socket_, data.data(), data.size());
#else
    ssize_t bytes_written = send(client_socket_, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0);
#endif
    if (bytes_written == -1 || bytes_written != static_cast<ssize_t>(data.size())) return false;

    return true;
}

void SocketClientHandler::Disconnected()
{
    connected_ = false;
}

} // namespace sercli
} // namespace nkhlab
