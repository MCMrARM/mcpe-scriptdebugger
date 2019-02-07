#include "Listener.h"

#include "InspectorManager.h"

namespace asio = boost::asio;
namespace beast = boost::beast;

using namespace InspectorServer;

Listener::Listener(HttpSessionHandler& httpSessionHandler) : tcpAcceptor(ioContext), tcpSocket(ioContext),
                                                             httpSessionHandler(httpSessionHandler) {
}

void Listener::start(unsigned short port) {
    auto guard = boost::asio::make_work_guard(ioContext);
    ioThread = std::thread([this]() {
        ioContext.run();
        printf("Exited IOContext\n");
    });


    tcp::endpoint endp {asio::ip::make_address("0.0.0.0"), port};
    beast::error_code ec;
    tcpAcceptor.open(endp.protocol(), ec);
    if (ec)
        abort();
    tcpAcceptor.set_option(asio::socket_base::reuse_address(true));
    tcpAcceptor.bind(endp, ec);
    if (ec)
        abort();
    tcpAcceptor.listen(10, ec);
    if (ec)
        abort();
    printf("Started on port %i\n", (int) port);
    doAccept();
}

void Listener::doAccept() {
    tcpAcceptor.async_accept(tcpSocket, [this](beast::error_code ec) {
        if (ec) {
            printf("Listener::doAccept: error\n");
            return;
        }
        std::make_shared<HttpSession>(std::move(tcpSocket), httpSessionHandler)->run();
        doAccept();
    });
}
