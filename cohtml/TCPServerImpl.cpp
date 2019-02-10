#include "TCPServerImpl.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "../Log.h"

TCPServerImpl::TCPServerImpl(cohtml::server::TCPListener &listener) : listener(listener) {
}

void TCPServerImpl::Run(unsigned short port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr))) {
        perror("bind");
        abort();
    }
    if (listen(sockfd, 10)) {
        perror("listen");
        abort();
    }
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

void TCPServerImpl::Update() {
    {
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        int fd = accept(sockfd, (struct sockaddr *) &addr, &addrlen);
        if (fd < 0) {
            if (errno != EAGAIN) {
                perror("accept");
                abort();
            }
        } else {
            Log::trace("CohtmlTcpServerImpl", "Connection opened (%i)", fd);
            listener.OnConnectionAccepted(fd);
            fcntl(fd, F_SETFL, O_NONBLOCK);
            clients.insert({fd, ClientInfo()});
        }
    }
    char buf[1024 * 16];
    for (auto& cd : clients) {
        int fd = cd.first;
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno != EAGAIN) {
                Log::trace("CohtmlTcpServerImpl", "Connection dropped (%i)", fd);
                listener.OnConnectionFailed(fd);
                clients.erase(fd);
                close(fd);
                continue;
            }
        } else if (n == 0) {
            Log::trace("CohtmlTcpServerImpl", "Connection closed (%i)", fd);
            listener.OnConnectionFailed(fd);
            clients.erase(fd);
            close(fd);
            continue;
        } else {
            listener.OnReadData(fd, buf, (unsigned int) n, false);
        }
        auto& ci = cd.second;
        if (ci.sendbuf.size() > ci.sendbufOff) {
            ssize_t res = WriteWithCheck(fd, &ci.sendbuf[ci.sendbufOff], ci.sendbuf.size() - ci.sendbufOff);
            if (res < 0)
                continue;
            ci.sendbufOff += res;
            if (ci.sendbufOff > 1024) {
                ci.sendbuf.erase(ci.sendbuf.begin(), ci.sendbuf.begin() + ci.sendbufOff);
                ci.sendbufOff = 0;
            }
        } else {
            if (ci.sendbufOff != 0) {
                ci.sendbuf.clear();
                ci.sendbufOff = 0;
            }
            if (ci.shouldClose) {
                close(fd);
                clients.erase(fd);
            }
        }
    }
}

ssize_t TCPServerImpl::WriteWithCheck(int fd, const char *data, size_t len) {
    ssize_t res = write(fd, data, len);
    if (res < 0) {
        if (errno == EAGAIN) {
            return 0;
        } else {
            Log::trace("CohtmlTcpServerImpl", "Connection dropped on write (%i)", fd);
            listener.OnConnectionFailed(fd);
            clients.erase(fd);
            return -1;
        }
    }
    return res;
}

void TCPServerImpl::SendData(int fd, const char *data, size_t len) {
    auto& client = clients.at(fd);
    ssize_t res = 0;
    if (client.sendbuf.size() == client.sendbufOff) {
        res = WriteWithCheck(fd, data, len);
        if (res < 0)
            return;
    }
    if (res > 0)
        client.sendbuf.insert(client.sendbuf.end(), data + res, data + len);
}

void TCPServerImpl::CloseConnection(int fd) {
    Log::trace("CohtmlTcpServerImpl", "Closing remote connection (%i)", fd);
    auto client = clients.find(fd);
    if (client != clients.end())
        client->second.shouldClose = true;
}