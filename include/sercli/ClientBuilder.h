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

#include "sercli/IClient.h"

namespace nkhlab {
namespace sercli {

using IClientPtr = std::unique_ptr<IClient>;

IClientPtr CreateUnixClient(const std::string& socket_path);
IClientPtr CreateInetClient(const std::string& address, int port);

} // namespace sercli
} // namespace nkhlab