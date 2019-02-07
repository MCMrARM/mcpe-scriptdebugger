#include "HttpSession.h"
#include "HttpSessionHandler.h"

namespace asio = boost::asio;
namespace beast = boost::beast;

using namespace InspectorServer;

void HttpSession::run() {
    beast::http::async_read(tcpSocket, buffer, req, std::bind(
            &HttpSession::onRead, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2));
}

void HttpSession::onRead(boost::beast::error_code ec, std::size_t) {
    if (ec == beast::http::error::end_of_stream) {
        tcpSocket.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }
    if (ec) {
        printf("HttpSession::onRead error\n");
        return;
    }
    handler.onRequest(*this, req);
}

void HttpSession::onWrite(boost::beast::error_code ec, bool close) {
    if (ec) {
        printf("HttpSession::onWrite error\n");
        return;
    }
    if (close) {
        tcpSocket.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    req = {};

    beast::http::async_read(tcpSocket, buffer, req, std::bind(
            &HttpSession::onRead, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2));
}