#include "../minecraft/Cohtml.h"

#include <set>

class TCPServerImpl : public cohtml::server::TCPServer {

private:
    cohtml::server::TCPListener& listener;
    int sockfd;
    std::set<int> clients;

public:
    TCPServerImpl(cohtml::server::TCPListener& listener);

    void Run(unsigned short port);

    void Update() override;

    void SendData(int fd, const char* data, size_t len) override;

    void CloseConnection(int fd) override;

};