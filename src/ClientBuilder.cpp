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

#include "SocketClient.h"
#include "sercli/ClientBuilder.h"

namespace nkhlab {
namespace sercli {

IClientPtr CreateUnixClient(const char* socket_path)
{
#ifdef __linux__
    return std::make_unique<SocketClient<UnixSocket>>(socket_path);
#else
#error "Unix socket connection is not supported!"
#endif
}

IClientPtr CreateInetClient(const char* address, int port)
{
    return std::make_unique<SocketClient<InetSocket>>(address, port);
}

} // namespace sercli
} // namespace nkhlab
