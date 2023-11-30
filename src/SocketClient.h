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

#include <atomic>
#include <map>
#include <thread>

#include "sercli/IClient.h"

#include "Constants.h"
#include "SmartSocket.h"

namespace nkhlab {
namespace sercli {

constexpr int kStopHandleTimeout_ms = 500;

template <class SocketT>
class SocketClient : public IClient
{
public:
    template <class... Args>
    SocketClient(const Args&... args)
        : smart_socket_{args...}
        , disconnected_{true}
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

    ~SocketClient() { Disconnect(); }

    bool Connect(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb) override
    {
        bool ret = false;

        smart_socket_.Start();

        if (smart_socket_.GetRawSocket() != kSocketError)
        {
            disconnected_ = false;
            worker_thread_ =
                std::thread(&SocketClient::Routine, this, server_disconnected_cb, data_received_cb);
            ret = true;
        }

        return ret;
    }

    void Disconnect() override
    {
        disconnected_ = true;
#ifdef __linux__
#else
        smart_socket_.ForceClose();
#endif
        if (worker_thread_.joinable()) worker_thread_.join();
    }

    bool Send(const std::vector<uint8_t>& data) override
    {
        if (disconnected_) return false;

#ifdef __linux__
        ssize_t bytes_written = write(smart_socket_.GetRawSocket(), data.data(), data.size());
#else
        ssize_t bytes_written = send(
            smart_socket_.GetRawSocket(),
            reinterpret_cast<const char*>(data.data()),
            static_cast<int>(data.size()),
            0);
#endif
        if (bytes_written == -1 || bytes_written != static_cast<ssize_t>(data.size())) return false;

        return true;
    }

private:
#ifdef __linux__
    void Routine(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb)
    {
        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            // Handle error
            disconnected_ = true;
        }

        epoll_event client_event;
        client_event.data.fd = smart_socket_.GetRawSocket();
        client_event.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, smart_socket_.GetRawSocket(), &client_event);
        constexpr int MAX_EVENTS = 10; // TODO: why?
        std::vector<epoll_event> events(MAX_EVENTS);

        while (!disconnected_)
        {
            int num_events = epoll_wait(
                epoll_fd,
                events.data(),
                MAX_EVENTS,
                kStopHandleTimeout_ms); // if pass __timeout as -1 - no timeout
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
                if (events[i].data.fd == smart_socket_.GetRawSocket())
                {
                    // Handle data from Server
                    std::vector<uint8_t> buffer(kDataBufferSize);

                    int received_bytes =
                        read(smart_socket_.GetRawSocket(), buffer.data(), buffer.size());
                    if (received_bytes == 0)
                    {
                        // Server disconnected
                        if (server_disconnected_cb) server_disconnected_cb();
                        disconnected_ = true;
                        break;
                        break;
                    }
                    else if (received_bytes < 0)
                    {
                        // Error occurred
                    }
                    else
                    {
                        // Handle received data
                        if (data_received_cb)
                        {
                            buffer.resize(received_bytes);
                            data_received_cb(buffer);
                        }
                    }
                }
            }
        }

        if (epoll_fd != -1) close(epoll_fd);
    }
#else
    void Routine(ServerDisconnectedCb server_disconnected_cb, ClientDataReceivedCb data_received_cb)
    {
        LPWSAOVERLAPPED wsa_recv_overlapped = &wsa_overlapped_;
        wsa_recv_overlapped->hEvent = WSACreateEvent();

        if (wsa_recv_overlapped->hEvent != nullptr)
        {
            while (!disconnected_)
            {
                SOCKET wsa_recv_sock = smart_socket_.GetRawSocket();
                LPWSABUF wsa_recv_buf = &wsa_receive_buf_;
                LPDWORD wsa_recv_flags = &wsa_receive_flags_;

                int result = WSARecv(
                    wsa_recv_sock, wsa_recv_buf, 1, nullptr, wsa_recv_flags, wsa_recv_overlapped, nullptr);

                if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
                {
                    if (server_disconnected_cb) server_disconnected_cb();
                    disconnected_ = true;
                }
                else
                {
                    if (WSAWaitForMultipleEvents(
                            1, &wsa_recv_overlapped->hEvent, true, INFINITE, true) != WSA_WAIT_FAILED)
                    {
                        DWORD wsa_received_bytes = 0;
                        if (WSAGetOverlappedResult(
                                wsa_recv_sock, wsa_recv_overlapped, &wsa_received_bytes, false, wsa_recv_flags))
                        {
                            if (wsa_received_bytes > 0)
                            {
                                if (data_received_cb)
                                {
                                    receive_buffer_.resize(wsa_received_bytes);
                                    data_received_cb(receive_buffer_);
                                    receive_buffer_.resize(kDataBufferSize);
                                }
                            }

                            WSAResetEvent(wsa_recv_overlapped->hEvent);
                        }
                    }
                }
            }
            WSACloseEvent(wsa_recv_overlapped->hEvent);
        }
    }

    WSAOVERLAPPED wsa_overlapped_;
    WSABUF wsa_receive_buf_;
    DWORD wsa_receive_flags_;
    std::vector<uint8_t> receive_buffer_;
#endif

    SmartSocket<Client, SocketT> smart_socket_;
    std::thread worker_thread_;
    std::atomic_bool disconnected_;
};

} // namespace sercli
} // namespace nkhlab
