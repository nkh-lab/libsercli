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
#include <memory>
#include <string>
#include <vector>

namespace nkhlab {
namespace sercli {

class IClientHandler;
using IClientHandlerPtr = std::shared_ptr<IClientHandler>;

class IClientHandler
{
public:
    virtual ~IClientHandler() = default;

    virtual const std::string& GetId() = 0;

    virtual bool IsConnected() = 0;
    virtual bool Send(const std::vector<uint8_t>& data) = 0;
};

using ClientStatusCb = std::function<void(IClientHandlerPtr client, bool connected)>;
using ServerDataReceivedCb =
    std::function<void(IClientHandlerPtr client, const std::vector<uint8_t>& data)>;

class IServer
{
public:
    virtual ~IServer() = default;

    virtual bool Start(ClientStatusCb client_status_cb, ServerDataReceivedCb server_data_received_cb) = 0;
    virtual void Stop() = 0;

    virtual std::vector<IClientHandlerPtr> GetClients() = 0;
    virtual IClientHandlerPtr GetClient(const std::string& id) = 0;
};

} // namespace sercli
} // namespace nkhlab