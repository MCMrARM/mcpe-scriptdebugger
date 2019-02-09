#pragma once

#include <v8-inspector.h>
#include <mutex>
#include <functional>
#include "WebSocketSession.h"

namespace InspectorServer {

class InspectorWebSocketSession : public v8_inspector::V8Inspector::Channel, public WebSocketSession {

private:
    std::mutex queue_mutex;
    std::vector<std::string> queue;
    std::unique_ptr<v8_inspector::V8InspectorSession> session;
    std::function<void ()> postMessageCb;
    std::function<void ()> closeCb;

protected:
    void onMessageReceived(std::string const &data) override {
        {
            std::lock_guard<std::mutex> g(queue_mutex);
            queue.push_back(std::move(data));
        }
        postMessageCb();
    }

public:
    explicit InspectorWebSocketSession(boost::asio::ip::tcp::socket s) : WebSocketSession(std::move(s)) {}

    ~InspectorWebSocketSession() override {
        if (closeCb)
            closeCb();
    }

    void setSession(std::unique_ptr<v8_inspector::V8InspectorSession> session) {
        this->session = std::move(session);
    }

    v8_inspector::V8InspectorSession* getSession() {
        return this->session.get();
    }

    void setPostMessageCallback(std::function<void ()> cb) {
        postMessageCb = std::move(cb);
    }
    void setCloseCallback(std::function<void ()> cb) {
        closeCb = std::move(cb);
    }

    void update();

    void sendInspectorMessage(const v8_inspector::StringView& msg);

    void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override {
        sendInspectorMessage(message->string());
    }

    void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override {
        sendInspectorMessage(message->string());
    }

    void flushProtocolNotifications() override {
    }
};

}