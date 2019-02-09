#include "TCPServerImpl.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

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
            printf("[Cohtml TcpServerImpl] Connection opened (%i)\n", fd);
            listener.OnConnectionAccepted(fd);
            fcntl(fd, F_SETFL, O_NONBLOCK);
            clients.insert(fd);
        }
    }
    char buf[1024 * 16];
    for (int fd : clients) {
        int n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EAGAIN)
                continue;
            printf("[Cohtml TcpServerImpl] Connection dropped (%i)\n", fd);
            listener.OnConnectionFailed(fd);
            clients.erase(fd);
            continue;
        }
        if (n == 0)
            continue;
        listener.OnReadData(fd, buf, (unsigned int) n, false);
    }
}

void TCPServerImpl::SendData(int fd, const char *data, size_t len) {
    write(fd, data, len);
}

void TCPServerImpl::CloseConnection(int fd) {
    if (clients.count(fd) > 0) {
        close(fd);
        clients.erase(fd);
    }
}