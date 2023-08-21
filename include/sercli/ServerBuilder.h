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

#include <memory>

#include "sercli/IServer.h"

namespace nkhlab {
namespace sercli {

using IServerPtr = std::unique_ptr<IServer>;

IServerPtr CreateUnixServer(const std::string& socket_path);
IServerPtr CreateInetServer(const std::string& address, int port);

} // namespace sercli
} // namespace nkhlab