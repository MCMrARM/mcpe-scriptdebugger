#include "WebSocketSession.h"

namespace beast = boost::beast;

using namespace InspectorServer;

void WebSocketSession::onAccept(boost::beast::error_code ec) {
    if (ec) {
        printf("WebSocketSession::onAccept: error\n");
        return;
    }
    ws.async_read(buffer, std::bind(&WebSocketSession::onRead, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2));
}

void WebSocketSession::onRead(boost::beast::error_code ec, std::size_t) {
    if (ec) {
        printf("WebSocketSession::onRead: error\n");
        return;
    }
    onMessageReceived(beast::buffers_to_string(buffer.data()));
    buffer.consume(buffer.size());

    ws.async_read(buffer, std::bind(&WebSocketSession::onRead, shared_from_this(),
                                    std::placeholders::_1, std::placeholders::_2));
}

void WebSocketSession::onWrite(boost::beast::error_code ec, std::size_t) {
    if (ec) {
        printf("WebSocketSession::onWrite: error\n");
        return;
    }
    std::lock_guard<std::mutex> lk (sendQueueMutex);
    sendQueue.pop();
    std::string send;
    if (!sendQueue.empty())
        ws.async_write(boost::asio::buffer(sendQueue.front()), std::bind(&WebSocketSession::onWrite, shared_from_this(),
                                                                         std::placeholders::_1, std::placeholders::_2));
}

void WebSocketSession::sendMessage(std::string data) {
    std::lock_guard<std::mutex> lk (sendQueueMutex);
    sendQueue.push(std::move(data));
    if (sendQueue.size() > 1)
        return;
    ws.async_write(boost::asio::buffer(sendQueue.front()), std::bind(&WebSocketSession::onWrite, shared_from_this(),
                                                                     std::placeholders::_1, std::placeholders::_2));
}
