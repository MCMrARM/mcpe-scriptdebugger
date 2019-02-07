#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace InspectorServer {

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {

private:
    using tcp = boost::asio::ip::tcp;

    boost::beast::flat_buffer buffer;
    boost::beast::websocket::stream<tcp::socket> ws;
    std::queue<std::string> sendQueue;
    std::mutex sendQueueMutex;

    void onAccept(boost::beast::error_code ec);

    void onRead(boost::beast::error_code ec, std::size_t);

    void onWrite(boost::beast::error_code ec, std::size_t);

protected:
    virtual void onMessageReceived(std::string const& data) = 0;

public:
    explicit WebSocketSession(tcp::socket s) : ws(std::move(s)) {}

    template <typename Body, typename Allocator>
    void run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req) {
        ws.async_accept(req, std::bind(&WebSocketSession::onAccept, shared_from_this(), std::placeholders::_1));
    }

    void sendMessage(std::string data);

};

}