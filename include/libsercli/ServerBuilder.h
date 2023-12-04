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

#include <memory>

#include "libsercli/IServer.h"

#ifdef __linux__
#define DLL_EXPORT
#else
#define DLL_EXPORT __declspec(dllexport)
#endif

namespace nkhlab {
namespace libsercli {

using IServerPtr = std::unique_ptr<IServer>;

IServerPtr DLL_EXPORT CreateUnixServer(const char* socket_path);
IServerPtr DLL_EXPORT CreateInetServer(const char* address, int port);

} // namespace libsercli
} // namespace nkhlab

#undef DLL_EXPORT