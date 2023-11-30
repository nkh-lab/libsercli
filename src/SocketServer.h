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

#ifdef __linux__
#include <sys/epoll.h>
#endif

#include <algorithm>
#include <atomic>
#include <iterator>
#include <map>
#include <mutex>
#include <thread>

#include "Constants.h"
#include "sercli/IServer.h"

#include "SmartSocket.h"

namespace nkhlab {
namespace sercli {

template <class SocketT>
class SocketServer;

template <class SocketT>
class SocketClientHandler : public IClientHandler
{
public:
    SocketClientHandler(SOCKET client_socket, SocketServer<SocketT>* server)
        : socket_{client_socket}
        , server_{server}
        , id_{std::to_string(client_socket)}
        , connected_{true}
    {
#ifdef __linux__
#else
        receive_buffer_.reserve(kDataBufferSize);
        receive_buffer_.resize(kDataBufferSize);
        wsa_receive_buf_.buf = reinterpret_cast<CHAR*>(receive_buffer_.data());
        wsa_receive_buf_.len = static_cast<ULONG>(receive_buffer_.size());
        wsa_receive_flags_ = 0;
        wsa_overlapped_ = {};
#endif
    }
    ~SocketClientHandler() = default;

    const std::string& GetId() override { return id_; }

    bool IsConnected() override { return connected_; }
    bool Send(const std::vector<uint8_t>& data) override
    {
        if (!connected_) return false;

#ifdef __linux__
        ssize_t bytes_written = write(socket_, data.data(), data.size());
#else
        ssize_t bytes_written = send(
            socket_, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0);
#endif
        if (bytes_written == -1 || bytes_written != static_cast<ssize_t>(data.size())) return false;

        return true;
    }

private:
#ifdef __linux__
#else
    //
    // WSAOVERLAPPED must be the first field because it is used in dereferencing
    // to access all members (for example, in a completition routine callback)
    //
    WSAOVERLAPPED wsa_overlapped_;
    WSABUF wsa_receive_buf_;
    DWORD wsa_receive_flags_;
    std::vector<uint8_t> receive_buffer_;
#endif
    const SOCKET socket_;
    SocketServer<SocketT>* server_;
    const std::string id_;
    std::atomic_bool connected_;

    friend class SocketServer<SocketT>;
};

template <class SocketT>
using SocketClientHandlerPtr = std::shared_ptr<SocketClientHandler<SocketT>>;

template <class SocketT>
class SocketServer : public IServer
{
public:
    template <class... Args>
    SocketServer(const Args&... args)
        : smart_socket_{args...}
        , stopped_{true}
    {
    }

    ~SocketServer() { Stop(); }

    bool Start(ClientStatusCb client_status_cb, ServerDataReceivedCb server_data_received_cb) override
    {
        bool ret = false;

        smart_socket_.Start();

        if (smart_socket_.GetRawSocket() != kSocketError)
        {
            stopped_ = false;
            worker_thread_ =
                std::thread(&SocketServer::Routine, this, client_status_cb, server_data_received_cb);
            ret = true;
        }

        return ret;
    }

    void Stop() override
    {
        stopped_ = true;

#ifdef __linux__
#else
        smart_socket_.ForceClose();
#endif
        if (worker_thread_.joinable()) worker_thread_.join();
    }

    std::vector<IClientHandlerPtr> GetClients() override
    {
        std::lock_guard<std::mutex> lk(clients_mtx_);

        std::vector<IClientHandlerPtr> clients;

        std::transform(clients_.begin(), clients_.end(), std::back_inserter(clients), [](auto& kv) {
            return kv.second;
        });

        return clients;
    }

    IClientHandlerPtr GetClient(const std::string& id) override
    {
        SOCKET socket;
        IClientHandlerPtr client = nullptr;

        try
        {
            socket = static_cast<int>(std::stoi(id));

            client = GetClient(socket);
        }
        catch (...)
        {
        }

        return client;
    }

private:
#ifdef __linux__
    void Routine(ClientStatusCb client_status_cb, ServerDataReceivedCb server_data_received_cb)
    {
        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            // Handle error
            stopped_ = true;
        }

        SOCKET server_socket = smart_socket_.GetRawSocket();
        epoll_event server_event;
        server_event.data.fd = server_socket;
        server_event.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &server_event);
        constexpr int MAX_EVENTS = 10; // TODO: why?
        constexpr int STOP_HANDLE_TIMEOUT_MS = 500;
        std::vector<epoll_event> events(MAX_EVENTS);

        while (!stopped_)
        {
            int num_events = epoll_wait(
                epoll_fd,
                events.data(),
                MAX_EVENTS,
                STOP_HANDLE_TIMEOUT_MS); // if pass __timeout as -1 - no timeout
            if (num_events == -1)
            {
                // Handle error
                break;
            }
            else if (num_events == 0)
            {
                // Timeout occurred, no events - we use it to handle stop request
            }

            for (int i = 0; i < num_events; ++i)
            {
                int client_socket;

                if (events[i].data.fd == server_socket)
                {
                    // New client connected
                    client_socket = accept(server_socket, nullptr, nullptr);
                    epoll_event client_event;
                    client_event.data.fd = client_socket;
                    client_event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event);

                    auto client = AddClient(client_socket);

                    if (client && client_status_cb) client_status_cb(client, true);
                }
                else
                {
                    // Handle data from existing clients
                    client_socket = events[i].data.fd;
                    std::vector<uint8_t> buffer(kDataBufferSize);

                    int bytes_read = read(client_socket, buffer.data(), buffer.size());
                    if (bytes_read == 0)
                    {
                        // Client disconnected
                        auto client = GetClient(client_socket);

                        if (client)
                        {
                            client->connected_ = false;
                            RemoveClient(client_socket);
                            if (client_status_cb) client_status_cb(client, false);
                        }

                        // Handle the disconnection
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                        close(client_socket);
                    }
                    else if (bytes_read < 0)
                    {
                        // Error occurred
                    }
                    else
                    {
                        // Handle received data
                        if (server_data_received_cb)
                        {
                            auto client = GetClient(client_socket);
                            buffer.resize(bytes_read);
                            server_data_received_cb(client, buffer);
                        }
                    }
                }
            }
        }

        if (epoll_fd != -1) close(epoll_fd);
    }
#else
    void Routine(ClientStatusCb client_status_cb, ServerDataReceivedCb server_data_received_cb)
    {
        client_status_cb_ = client_status_cb;
        server_data_received_cb_ = server_data_received_cb;

        while (!stopped_)
        {
            SOCKET client_socket = accept(smart_socket_.GetRawSocket(), nullptr, nullptr);
            if (client_socket != kSocketError)
            {
                auto client = AddClient(static_cast<int>(client_socket));

                if (client && client_status_cb) client_status_cb(client, true);

                //
                // int WSAAPI WSARecv(
                //     [in]      SOCKET                             s,
                //     [in, out] LPWSABUF                           lpBuffers,
                //     [in]      DWORD                              dwBufferCount,
                //     [out]     LPDWORD                            lpNumberOfBytesRecvd,
                //     [in, out] LPDWORD                            lpFlags,
                //     [in]      LPWSAOVERLAPPED                    lpOverlapped,
                //     [in]      LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
                // );
                //
                SOCKET wsa_recv_sock = client_socket;
                LPWSABUF wsa_recv_buf = &client->wsa_receive_buf_;
                LPDWORD wsa_recv_flags = &client->wsa_receive_flags_;
                LPWSAOVERLAPPED wsa_recv_overlapped = &client->wsa_overlapped_;

                int result = WSARecv(
                    wsa_recv_sock,
                    wsa_recv_buf,
                    1,
                    nullptr,
                    wsa_recv_flags,
                    wsa_recv_overlapped,
                    &SocketServer<SocketT>::WsaReceiveCompletitionRoutine);

                if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
                {
                    client->connected_ = false;
                    RemoveClient(static_cast<int>(client_socket));
                    if (client_status_cb_) client_status_cb_(client, false);
                }
            }
        }
    }

    static void WsaReceiveCompletitionRoutine(
        DWORD error,
        DWORD received_bytes,
        LPWSAOVERLAPPED overlapped,
        DWORD flags)
    {
        UNUSED(flags);

        SocketClientHandler<SocketT>* rc =
            CONTAINING_RECORD(overlapped, SocketClientHandler<SocketT>, wsa_overlapped_);
        SOCKET client_socket = rc->socket_;

        auto client = rc->server_->GetClient(rc->socket_);
        if (!client) return;
        auto& server = client->server_;

        if (!server->stopped_ && client->connected_)
        {
            if (error != 0)
            {
                client->connected_ = false;
                server->RemoveClient(client_socket);
                if (server->client_status_cb_) server->client_status_cb_(client, false);
            }
            else
            {
                if (server->server_data_received_cb_)
                {
                    client->receive_buffer_.resize(received_bytes);
                    server->server_data_received_cb_(client, client->receive_buffer_);
                    client->receive_buffer_.resize(kDataBufferSize);
                }

                // next receiving
                SOCKET wsa_recv_sock = client_socket;
                LPWSABUF wsa_recv_buf = &client->wsa_receive_buf_;
                LPDWORD wsa_recv_flags = &client->wsa_receive_flags_;
                LPWSAOVERLAPPED wsa_recv_overlapped = &client->wsa_overlapped_;

                int result = WSARecv(
                    wsa_recv_sock,
                    wsa_recv_buf,
                    1,
                    nullptr,
                    wsa_recv_flags,
                    wsa_recv_overlapped,
                    &SocketServer<SocketT>::WsaReceiveCompletitionRoutine);

                if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
                {
                    client->connected_ = false;
                    server->RemoveClient(static_cast<int>(client_socket));
                    if (server->client_status_cb_) server->client_status_cb_(client, false);
                }
            }
        }
    }
#endif
    SocketClientHandlerPtr<SocketT> GetClient(SOCKET socket)
    {
        std::lock_guard<std::mutex> lk(clients_mtx_);

        auto it = clients_.find(socket);

        if (it != clients_.end())
            return (*it).second;
        else
            return nullptr;
    }

    SocketClientHandlerPtr<SocketT> AddClient(SOCKET socket)
    {
        std::lock_guard<std::mutex> lk(clients_mtx_);

        auto client = std::make_shared<SocketClientHandler<SocketT>>(socket, this);

        auto emplace_res = clients_.emplace(socket, client);

        if (emplace_res.second)
            return client;
        else
            return nullptr;
    }

    void RemoveClient(SOCKET socket)
    {
        std::lock_guard<std::mutex> lk(clients_mtx_);
        clients_.erase(socket);
    }

    SmartSocket<Server, SocketT> smart_socket_;
    std::atomic_bool stopped_;
    std::map<SOCKET, SocketClientHandlerPtr<SocketT>> clients_;
    std::mutex clients_mtx_;
    std::thread worker_thread_;
#ifdef __linux__
#else
    ClientStatusCb client_status_cb_;
    ServerDataReceivedCb server_data_received_cb_;
#endif
};

} // namespace sercli
} // namespace nkhlab
