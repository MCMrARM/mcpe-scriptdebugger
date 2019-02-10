#include "../minecraft/Cohtml.h"

#include <set>
#include <vector>
#include <map>

class TCPServerImpl : public cohtml::server::TCPServer {

private:
    struct ClientInfo {
        std::vector<char> sendbuf;
        size_t sendbufOff = 0;
        bool shouldClose = false;
    };

    cohtml::server::TCPListener& listener;
    int sockfd;
    std::map<int, ClientInfo> clients;

    ssize_t WriteWithCheck(int fd, const char* data, size_t len);

public:
    TCPServerImpl(cohtml::server::TCPListener& listener);

    void Run(unsigned short port);

    void Update() override;

    void SendData(int fd, const char* data, size_t len) override;

    void CloseConnection(int fd) override;

};