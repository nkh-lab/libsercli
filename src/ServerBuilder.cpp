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

#include <memory>

#include "SocketServer.h"
#include "sercli/ServerBuilder.h"

namespace nkhlab {
namespace sercli {

IServerPtr CreateUnixServer(const std::string& socket_path)
{
    return std::make_unique<SocketServer>(socket_path);
}

IServerPtr CreateInetServer(const std::string& address, int port)
{
    return nullptr;
}

} // namespace sercli
} // namespace nkhlab