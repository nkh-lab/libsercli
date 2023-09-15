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

namespace nkhlab {
namespace sercli {

class SocketBuilder
{
public:
    SocketBuilder() = delete;

    static int InitSocketForUnixServer(const char* unix_socket_path);
    static int InitSocketForInetServer(const char* inet_address, int inet_port);
    static int InitSocketForUnixClient(const char* unix_socket_path);
    static int InitSocketForInetClient(const char* inet_address, int inet_port);

private:
    static int InitSocket(
        bool is_server,
        bool is_unix,
        const char* unix_socket_path,
        const char* inet_address,
        int inet_port);
};

} // namespace sercli
} // namespace nkhlab