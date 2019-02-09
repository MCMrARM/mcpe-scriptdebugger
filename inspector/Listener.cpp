#include "Listener.h"

#include "InspectorManager.h"

#include "../Log.h"

namespace asio = boost::asio;
namespace beast = boost::beast;

using namespace InspectorServer;

Listener::Listener(HttpSessionHandler& httpSessionHandler) : tcpAcceptor(ioContext), tcpSocket(ioContext),
                                                             httpSessionHandler(httpSessionHandler) {
}

Listener::~Listener() {
    ioContext.stop();
    ioThread.join();
}

void Listener::start(unsigned short port) {
    auto guard = boost::asio::make_work_guard(ioContext);
    ioThread = std::thread([this]() {
        ioContext.run();
        Log::trace("Listener", "Exited IOContext");
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
    Log::info("Listener", "Started on port %i", port);
    doAccept();
}

void Listener::doAccept() {
    tcpAcceptor.async_accept(tcpSocket, [this](beast::error_code ec) {
        if (ec) {
            Log::trace("Listener", "doAccept: error %s", ec.message().c_str());
            return;
        }
        std::make_shared<HttpSession>(std::move(tcpSocket), httpSessionHandler)->run();
        doAccept();
    });
}
