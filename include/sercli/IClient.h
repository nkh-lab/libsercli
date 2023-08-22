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

#include <functional>
#include <string>
#include <vector>

namespace nkhlab {
namespace sercli {

using ServerDisconnectedCb = std::function<void()>;
using ClientDataReceivedCb = std::function<void(const std::vector<uint8_t>& data)>;

class IClient
{
public:
    virtual ~IClient() = default;

    virtual bool Connect(
        ServerDisconnectedCb server_disconnected_cb,
        ClientDataReceivedCb data_received_cb) = 0;
    virtual void Disconnect() = 0;

    virtual bool Send(const std::vector<uint8_t>& data) = 0;
};

} // namespace sercli
} // namespace nkhlab