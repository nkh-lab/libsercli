/*
 * Copyright (C) 2023 https://github.com/nkh-lab
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

#ifdef __linux__
#define DLL_EXPORT
#else
#define DLL_EXPORT __declspec(dllexport)
#endif

namespace nkhlab {
namespace libsercli {

using ServerDisconnectedCb = std::function<void()>;
using ClientDataReceivedCb = std::function<void(const std::vector<uint8_t>& data)>;

class DLL_EXPORT IClient
{
public:
    virtual ~IClient() = default;

    virtual bool Connect(
        ServerDisconnectedCb server_disconnected_cb,
        ClientDataReceivedCb data_received_cb) = 0;
    virtual void Disconnect() = 0;

    virtual bool Send(const std::vector<uint8_t>& data) = 0;
};

} // namespace libsercli
} // namespace nkhlab

#undef DLL_EXPORT