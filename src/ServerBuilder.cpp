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

#include <memory>

#include "Macros.h"
#include "SocketServer.h"
#include "libsercli/ServerBuilder.h"

namespace nkhlab {
namespace libsercli {

IServerPtr CreateUnixServer(const char* socket_path)
{
#ifdef __linux__
    return std::make_unique<SocketServer<UnixSocket>>(socket_path);
#else
    UNUSED(socket_path);
    return nullptr;
#endif
}

IServerPtr CreateInetServer(const char* address, int port)
{
    return std::make_unique<SocketServer<InetSocket>>(address, port);
}

} // namespace libsercli
} // namespace nkhlab