#pragma once

#include "WebSockets.h"
#include <v8-inspector.h>

class InspectorWebSocketChannel : public v8_inspector::V8Inspector::Channel {

private:
    using Connection = SimpleWeb::SocketServer<SimpleWeb::WS>::Connection;
    std::shared_ptr<Connection> connection;
    std::mutex queue_mutex;
    std::vector<std::string> queue;
    std::unique_ptr<v8_inspector::V8InspectorSession> session;
    std::function<void ()> postMessageCb;

public:
    explicit InspectorWebSocketChannel(std::shared_ptr<Connection> connection) : connection(std::move(connection)) {}

    std::shared_ptr<Connection> getConnection() const {
        return connection;
    }

    void setSession(std::unique_ptr<v8_inspector::V8InspectorSession> session) {
        this->session = std::move(session);
    }

    void setPostMessageCallback(std::function<void ()> cb) {
        postMessageCb = std::move(cb);
    }

    void postMessage(std::string data) {
        {
            std::lock_guard<std::mutex> g(queue_mutex);
            queue.push_back(std::move(data));
        }
        postMessageCb();
    }

    void update();

    void sendMessage(const v8_inspector::StringView& msg);

    void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override {
        sendMessage(message->string());
    }

    void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override {
        sendMessage(message->string());
    }

    void flushProtocolNotifications() override {
    }
};