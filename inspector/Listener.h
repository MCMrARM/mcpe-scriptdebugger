#pragma once

#include <thread>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "HttpSessionHandler.h"

namespace InspectorServer {

class Listener {

private:
    using tcp = boost::asio::ip::tcp;

    HttpSessionHandler& httpSessionHandler;
    boost::asio::io_context ioContext;
    tcp::acceptor tcpAcceptor;
    tcp::socket tcpSocket;
    std::thread ioThread;


    void doAccept();

public:
    Listener(HttpSessionHandler& httpSessionHandler);

    void start(unsigned short port);

};

}